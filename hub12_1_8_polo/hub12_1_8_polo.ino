// =================================================================
// CÓDIGO FINAL INTEGRADO: TIMER CÍCLICO CON MATRIZ LED HUB12 (ESP32)
// MODIFICACIÓN: Nuevo estado PIN_ACTIVO_3 al final del ciclo
// =================================================================

#include "soc/gpio_struct.h"
#include "letras.h" // Se asume que este archivo contiene la función getGlyph()

// --- DEFINICIONES DE PINES DE LA MATRIZ HUB12 (ESP32) ---
#define oe_pin      13
#define a_pin       12
#define b_pin       14
#define c_pin       33
#define clk_pin     27
#define sclk_pin    26
#define dato_pinR   25

#define FILAS 16
#define COLS  64

// --- DEFINICIONES DE PINES DEL TIMER ---
const int PIN_SALIDA = 2;       // Pin de salida para activar (usar un pin libre, ej: GPIO2)
const int PIN_BTN_ADELANTAR = 4;  // Pin del botón para adelantar/subir tiempo (ej: GPIO4)
const int PIN_BTN_RETRASAR = 5;   // Pin del botón para retrasar/bajar tiempo (ej: GPIO5)

// --- VARIABLES Y ESTRUCTURAS DE LA MATRIZ ---
#define MAX_COLS 80 // Suficiente para M:SS y el parpadeo
uint8_t matrizGlobal[FILAS][MAX_COLS];
int textoAncho = 0;
int fila = 1;
uint8_t bits[FILAS][COLS]; // Buffer de la matriz

// Buffers internos para el multiplexado de 1/8 scan
uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16];

// --- VARIABLES Y ESTRUCTURAS DEL TIMER CÍCLICO ---

// Estados del Ciclo (NUEVO ESTADO: PIN_ACTIVO_3)
enum EstadoCiclo {
    SIETE_MINUTOS,
    PIN_ACTIVO_1,
    TREINTA_SEGUNDOS,
    PIN_ACTIVO_2,
    TRES_MINUTOS,
    PIN_ACTIVO_3 // Nuevo estado: Pin HIGH por 3s antes de reiniciar
};

EstadoCiclo estadoActual = SIETE_MINUTOS;
unsigned long tiempoTotalCiclo = 0;
unsigned long tiempoInicioPaso = 0;
unsigned long tiempoRestante = 0;

// Variables de Temporización y Parpadeo
unsigned long ultimoSegundo = 0;
unsigned long ultimoParpadeo = 0;
const unsigned long INTERVALO_SEGUNDO = 1000;
const unsigned long INTERVALO_PARPADEO = 500;
bool puntosVisibles = true; // Estado de los dos puntos

// Variables de Control de Botones
unsigned long ultimoReboteAdelantar = 0;
unsigned long ultimoReboteRetrasar = 0;
const unsigned long TIEMPO_DEBOUNCE = 50;

// =================================================================
// --- FUNCIONES DE MANEJO DE LA MATRIZ LED (REUTILIZADAS Y SIMPLIFICADAS) ---
// =================================================================

// Función auxiliar para copiar submatrices (la lógica de la HUB12)
void copySubMatrix(uint8_t dst[8][16], uint8_t src[FILAS][COLS], int startRow, int startCol) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 16; c++) {
            dst[r][c] = src[startRow + r][startCol + c];
        }
    }
}

// Copia las 8 submatrices de 8x16 (una para cada fila par/impar de 4 paneles)
void subCopiar() {
    copySubMatrix(uno,   bits, 8,  0);
    copySubMatrix(dos,   bits, 0,  0);
    copySubMatrix(tres,  bits, 8, 16);
    copySubMatrix(cuatro,bits, 0, 16);
    copySubMatrix(cinco, bits, 8, 32);
    copySubMatrix(seis,  bits, 0, 32);
    copySubMatrix(siete, bits, 8, 48);
    copySubMatrix(ocho,  bits, 0, 48);
}

// Control de filas (A, B, C)
void eligeFila(bool a, bool b, bool c) {
    fastWrite(a_pin, a);
    fastWrite(b_pin, b);
    fastWrite(c_pin, c);
}

// Fast GPIO (usando registros del ESP32 para velocidad)
inline void fastWrite(uint8_t pin, bool val) {
    if (pin < 32) {
        uint32_t mask = (1UL << pin);
        if (val) GPIO.out_w1ts = mask;
        else     GPIO.out_w1tc = mask;
    } else {
        uint32_t mask = (1UL << (pin - 32));
        if (val) GPIO.out1_w1ts.val = mask;
        else     GPIO.out1_w1tc.val = mask;
    }
}

// Transfiere datos al shift register de la matriz
void escribe() {
    subCopiar();
    uint8_t (*subs[8])[16] = {ocho, siete, seis, cinco, cuatro, tres, dos, uno};
    int idxFila = fila - 1; // 0 a 7

    for (int s = 0; s < 8; s++) { // Para los 8 paneles
        uint8_t (*sub)[16] = subs[s];
        for (int j = 15; j >= 0; j--) { // 16 columnas
            fastWrite(dato_pinR, sub[idxFila][j]); 
            fastWrite(clk_pin, 1);
            fastWrite(clk_pin, 0);
        }
    }

    fastWrite(oe_pin, 1); // Desactiva la salida mientras se cambia la fila
    fastWrite(sclk_pin, 1); // Latch (Muestra los datos cargados)

    switch (fila) { // Elige la fila a encender
        case 1: eligeFila(1, 1, 1); break;
        case 2: eligeFila(0, 1, 1); break;
        case 3: eligeFila(1, 0, 1); break;
        case 4: eligeFila(0, 0, 1); break;
        case 5: eligeFila(1, 1, 0); break;
        case 6: eligeFila(0, 1, 0); break;
        case 7: eligeFila(1, 0, 0); break;
        case 8: eligeFila(0, 0, 0); break;
    }

    fastWrite(sclk_pin, 0); // Latch a LOW
    fastWrite(oe_pin, 0); // Activa la salida
    
    // Siguiente fila
    fila++;
    if (fila == 9) fila = 1;
}

// Copia la matriz global (el texto) al buffer 'bits' que se muestra
void copiar_a_bits(uint8_t bits[FILAS][COLS], int offset) {
    int offsetFijo = 0; 
    memset(bits, 0, FILAS * COLS);

    for (int r = 0; r < FILAS; r++) {
        for (int c = 0; c < COLS; c++) {
            int srcCol = c + offsetFijo;
            if (srcCol < MAX_COLS) {
                bits[r][c] = matrizGlobal[r][srcCol];
            }
        }
    }
}

/**
 * Dibuja un texto en la matrizGlobal. Ahora usado para dibujar el tiempo.
 */
void write_text(const char *text) {
    memset(matrizGlobal, 0, sizeof(matrizGlobal));

    const int charWidth = 8;
    const int textHeight = 8;//si es 16 y repeatCount es 2, estira el texto verticalmente 
    const int vOffset = (FILAS - textHeight) / 2; // Centrado vertical

    int len = strlen(text);
    textoAncho = len * charWidth;
    
    int hOffset = (COLS - textoAncho) / 2;
    if (hOffset < 0) hOffset = 0; 

    for (int t = 0; t < len; t++) {
        const uint8_t *glyph = getGlyph(text[t]);
        
        for (int row = 0; row < 8; row++) {
            uint8_t bitsRow = glyph[row];
            int repeatCount = 1; //si es 2 y textHeight 16 estira el texto verticalmente 
            
            for (int repeat = 0; repeat < repeatCount; repeat++) {
                int r = row * repeatCount + repeat + vOffset;
                if (r < 0 || r >= FILAS) continue;
                
                for (int bit = 0; bit < 8; bit++) {
                    int c = hOffset + t * charWidth + bit; 
                    if (c >= MAX_COLS) continue;
                    
                    matrizGlobal[r][c] = (bitsRow & (1 << (7 - bit))) ? 1 : 0;
                }
            }
        }
    }
}

// =================================================================
// --- FUNCIONES DE LÓGICA DEL TIMER Y CICLO ---
// =================================================================

void avanzarPaso();
void retrocederPaso();

/**
 * Establece la duración del paso actual, la salida y el tiempo de inicio.
 */
void inicializarPaso() {
    // 1. Inicializar la salida (Apagada por defecto para la cuenta)
    digitalWrite(PIN_SALIDA, LOW);
    
    // 2. Definir el tiempo del ciclo
    switch (estadoActual) {
        case SIETE_MINUTOS:
            tiempoTotalCiclo = 7 * 60 * 1000; // 7:00
            break;
        case PIN_ACTIVO_1:
            tiempoTotalCiclo = 3 * 1000; // 3 segundos (Activación del Pin)
            digitalWrite(PIN_SALIDA, HIGH); // ACTIVA EL PIN
            break;
        case TREINTA_SEGUNDOS:
            tiempoTotalCiclo = 30 * 1000; // 30 segundos
            break;
        case PIN_ACTIVO_2:
            tiempoTotalCiclo = 3 * 1000; // 3 segundos (Activación del Pin)
            digitalWrite(PIN_SALIDA, HIGH); // ACTIVA EL PIN
            break;
        case TRES_MINUTOS:
            tiempoTotalCiclo = 3 * 60 * 1000; // 3:00
            break;
        case PIN_ACTIVO_3: // ¡NUEVO ESTADO!
            tiempoTotalCiclo = 3 * 1000; // 3 segundos (Activación del Pin Final)
            digitalWrite(PIN_SALIDA, HIGH); // ACTIVA EL PIN
            break;
    }
    
    // 3. Reiniciar marcas de tiempo
    tiempoInicioPaso = millis();
    tiempoRestante = tiempoTotalCiclo;
}

/**
 * Muestra el tiempo restante en formato M:SS, incluyendo el parpadeo de los dos puntos.
 */
void mostrarTiempo(unsigned long ms) {
    long segundosTotales = ms / 1000;
    int minutos = segundosTotales / 60;
    int segundos = segundosTotales % 60;

    // Formato M:SS
    char buffer[6]; 
    
    // El carácter ':' parpadea cada 500ms
    char separador = puntosVisibles ? ':' : ' ';
    
    if (minutos < 0) minutos = 0;

    snprintf(buffer, sizeof(buffer), "%d%c%02d", minutos, separador, segundos);
    
    // Dibuja el texto en la matriz global
    write_text(buffer);
}

/**
 * Gestiona el paso del tiempo, la actualización del display y el parpadeo.
 */
void actualizarTimer() {
    unsigned long tiempoActual = millis();
    
    // --- Lógica del Ciclo (Terminación) ---
    if (tiempoActual - tiempoInicioPaso >= tiempoTotalCiclo) {
        avanzarPaso();
        return;
    }

    // --- Lógica de Temporización Precisa (1 segundo) ---
    if (tiempoActual - ultimoSegundo >= INTERVALO_SEGUNDO) {
        tiempoRestante = tiempoTotalCiclo - (tiempoActual - tiempoInicioPaso);
        if (tiempoRestante < 0) tiempoRestante = 0;
        
        // La actualización de los segundos es precisa
        ultimoSegundo = tiempoActual;
    }
    
    // --- Lógica de Parpadeo de los Dos Puntos (500ms) ---
    if (tiempoActual - ultimoParpadeo >= INTERVALO_PARPADEO) {
        puntosVisibles = !puntosVisibles; // Invierte el estado
        ultimoParpadeo = tiempoActual;
    }

    // Muestra el tiempo restante con el estado de parpadeo actual
    mostrarTiempo(tiempoRestante);
}

// --- FUNCIONES DE CONTROL DE CICLO (Actualizadas) ---

void avanzarPaso() {
    switch (estadoActual) {
        case SIETE_MINUTOS:    estadoActual = PIN_ACTIVO_1;     break;
        case PIN_ACTIVO_1:     estadoActual = TREINTA_SEGUNDOS; break;
        case TREINTA_SEGUNDOS: estadoActual = PIN_ACTIVO_2;     break;
        case PIN_ACTIVO_2:     estadoActual = TRES_MINUTOS;     break;
        case TRES_MINUTOS:     estadoActual = PIN_ACTIVO_3;     break; // Transición a nuevo estado
        case PIN_ACTIVO_3:     estadoActual = SIETE_MINUTOS;    break; // Vuelve a empezar
    }
    inicializarPaso();
}

void retrocederPaso() {
    switch (estadoActual) {
        case SIETE_MINUTOS:    estadoActual = PIN_ACTIVO_3;     break; // Al inicio, retrocede al final
        case PIN_ACTIVO_1:     estadoActual = SIETE_MINUTOS;    break;
        case TREINTA_SEGUNDOS: estadoActual = PIN_ACTIVO_1;     break;
        case PIN_ACTIVO_2:     estadoActual = TREINTA_SEGUNDOS; break;
        case TRES_MINUTOS:     estadoActual = PIN_ACTIVO_2;     break;
        case PIN_ACTIVO_3:     estadoActual = TRES_MINUTOS;     break; // Retrocede de Pin3 a 3 min
    }
    inicializarPaso();
}

void resetearCiclo() {
    estadoActual = SIETE_MINUTOS;
    inicializarPaso();
}

// --- FUNCIONES DE GESTIÓN DE BOTONES ---

/**
 * Lee los botones con Debounce y ejecuta las acciones (cambio de paso o reset).
 */
void gestionarBotones() {
    unsigned long tiempoActual = millis();

    // Lectura de los botones (INPUT_PULLUP, LOW = presionado)
    bool btnAdelantarPresionado = (digitalRead(PIN_BTN_ADELANTAR) == LOW);
    bool btnRetrasarPresionado = (digitalRead(PIN_BTN_RETRASAR) == LOW);

    // 1. Lógica de Reseteo (Ambos botones al mismo tiempo)
    if (btnAdelantarPresionado && btnRetrasarPresionado) {
        if (tiempoActual - ultimoReboteAdelantar > TIEMPO_DEBOUNCE && 
            tiempoActual - ultimoReboteRetrasar > TIEMPO_DEBOUNCE) {
            resetearCiclo();
            ultimoReboteAdelantar = tiempoActual;
            ultimoReboteRetrasar = tiempoActual;
        }
        return;
    }

    // 2. Lógica del Botón Adelantar (Avanza al siguiente paso)
    if (btnAdelantarPresionado) {
        if (tiempoActual - ultimoReboteAdelantar > TIEMPO_DEBOUNCE) {
            avanzarPaso(); 
            ultimoReboteAdelantar = tiempoActual;
        }
    }

    // 3. Lógica del Botón Retrasar (Retrocede al paso anterior)
    if (btnRetrasarPresionado) {
        if (tiempoActual - ultimoReboteRetrasar > TIEMPO_DEBOUNCE) {
            retrocederPaso();
            ultimoReboteRetrasar = tiempoActual;
        }
    }
}

// =================================================================
// --- SETUP y LOOP ---
// =================================================================

void setup() {
    // Inicialización de pines de la matriz
    pinMode(oe_pin, OUTPUT);
    pinMode(a_pin, OUTPUT);
    pinMode(b_pin, OUTPUT);
    pinMode(c_pin, OUTPUT);
    pinMode(clk_pin, OUTPUT);
    pinMode(sclk_pin, OUTPUT);
    pinMode(dato_pinR, OUTPUT);

    // Inicialización de pines del timer y botones
    pinMode(PIN_SALIDA, OUTPUT);
    digitalWrite(PIN_SALIDA, LOW); 
    
    pinMode(PIN_BTN_ADELANTAR, INPUT_PULLUP); 
    pinMode(PIN_BTN_RETRASAR, INPUT_PULLUP);

    // Valores iniciales de la matriz
    fastWrite(oe_pin, 0);
    fastWrite(a_pin, 0);
    fastWrite(b_pin, 0);
    fastWrite(c_pin, 0);
    fastWrite(sclk_pin, 0);
    fastWrite(clk_pin, 0);
    fastWrite(dato_pinR, 0);

    // Iniciar el timer en el primer paso (7:00)
    inicializarPaso();
}

void loop() {
    // 1. Lógica del Timer y Actualización de la Matriz Global
    actualizarTimer();
    
    // 2. Gestión de Entradas (Botones)
    gestionarBotones();
    
    // 3. Transferencia a la Matriz Física (multiplexado)
    copiar_a_bits(bits, 0);
    escribe();
}
