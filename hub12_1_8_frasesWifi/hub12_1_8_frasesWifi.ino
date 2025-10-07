// P4.75 HUB12 1/8 scan 16x64 de un solo color, matrix control.
// scroll text maximo MAX_COLS, caracteres de 8 bits de ancho

#include "soc/gpio_struct.h"
#include <WiFi.h>           // Para conexión Wi-Fi
#include <HTTPClient.h>     // Para hacer solicitudes HTTP (descargar frases)
#include "letras.h" // Se asume que este archivo contiene getGlyph() y la font data

// ===== CONFIGURACIÓN DE PINES (ESP32 dev module) =====
#define oe_pin      13
#define a_pin       12
#define b_pin       14
#define c_pin       33
#define clk_pin     27
#define sclk_pin    26
#define dato_pinR   25

#define FILAS 16
#define COLS  64

// ===== CREDENCIALES Y CONSTANTES DE RED =====
const char* ssid = "mosqueton";
const char* password = "esmerilemelo";

// Buffer y constantes para almacenar la frase descargada
#define MAX_QUOTE_LENGTH 440
char currentQuote[MAX_QUOTE_LENGTH] = "   Inicializando matriz. Conectando a internet...   ";

// URL de una API de frases (ejemplo: Forismatic API, devuelve texto simple)
// Si esta API falla o devuelve JSON, necesitarás ajustarla o usar ArduinoJson.
const char* QUOTE_API_URL = "http://api.forismatic.com/api/1.0/?method=getQuote&lang=en&format=text"; 

// Control de tiempo para la actualización (4 horas en milisegundos)
unsigned long lastUpdateMillis = 0;
//const long updateInterval = 4UL * 60UL * 60UL * 1000UL;
const long updateInterval = 10UL * 60UL * 1000UL; //10 minutos

// ===== VARIABLES GLOBALES DE SCROLL Y MATRIZ =====
#define MAX_CHARS_SUPPORTED 400
#define MAX_COLS (MAX_CHARS_SUPPORTED * 8) // Ahora son 1600 píxeles de ancho.

uint8_t matrizGlobal[FILAS][MAX_COLS];  // Matriz de 16x640 para el texto completo
int textoAncho = 0;   // Ancho real del texto dibujado en píxeles
int fila  = 1;
int refresh = 300;    // Velocidad de scroll (menor = más rápido)
int aux   = 0;
int offset  = 0;
uint8_t bits[FILAS][COLS]; // Buffer de visualización (16x64)

// Buffers para las 8 secciones de 8x16 de la matriz de visualización
uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16];

// Variable que apunta al texto a mostrar. Usamos el buffer dinámico.
const char *TEXTO_A_MOSTRAR = currentQuote; 

// Se asume que esta función existe en letras.h o en otro lugar
// void copySubMatrix(uint8_t dest[FILAS][COLS], uint8_t src[8][16], int row_offset, int col_offset); 

// =========================================================
//                  FUNCIONES DE GRÁFICOS Y HW
// =========================================================

void subCopiar() {
  // Nota: Estas llamadas asumen que 'copySubMatrix' está definida para mover
  // los datos de 'bits' (16x64) a los 8 sub-buffers (8x16).
  // Si no tienes 'copySubMatrix', esta función fallará.
  
  copySubMatrix(bits, uno,    8,  0);
  copySubMatrix(bits, dos,    0,  0);
  copySubMatrix(bits, tres,   8, 16);
  copySubMatrix(bits, cuatro, 0, 16);
  copySubMatrix(bits, cinco,  8, 32);
  copySubMatrix(bits, seis,   0, 32);
  copySubMatrix(bits, siete,  8, 48);
  copySubMatrix(bits, ocho,   0, 48);
}

void eligeFila(bool a,bool b,bool c) {
  fastWrite(a_pin,a);
  fastWrite(b_pin,b);
  fastWrite(c_pin,c);
}

// ===== Fast GPIO =====
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

void escribe() {
  subCopiar();
  // El resto de esta función maneja el control del HUB12
  uint8_t (*subs[8])[16] = {ocho, siete, seis, cinco, cuatro, tres, dos, uno};
  int idxFila = fila - 1;
  if (idxFila < 0) idxFila = 0;
  if (idxFila > 7) idxFila = 7;

  for (int s = 0; s < 8; s++) {
    uint8_t (*sub)[16] = subs[s];
    for (int j = 15; j >= 0; j--) {
      fastWrite(dato_pinR, sub[idxFila][j]);
      fastWrite(clk_pin, 1);
      fastWrite(clk_pin, 0);
    }
  }

  fastWrite(oe_pin, 1);
  fastWrite(sclk_pin, 1);

  switch (fila) {
    case 1: eligeFila(1,1,1); break; // Fila 8 (o R0)
    case 2: eligeFila(0,1,1); break; // Fila 7
    case 3: eligeFila(1,0,1); break; // Fila 6
    case 4: eligeFila(0,0,1); break; // Fila 5
    case 5: eligeFila(1,1,0); break; // Fila 4
    case 6: eligeFila(0,1,0); break; // Fila 3
    case 7: eligeFila(1,0,0); break; // Fila 2
    case 8: eligeFila(0,0,0); break; // Fila 1 (o R7)
  }

  fastWrite(sclk_pin, 0);
  fastWrite(oe_pin, 0);

  fila++;
  if (fila == 9) fila = 1;
}

void copiar_a_bits(uint8_t bits[FILAS][COLS], int offset) {
    memset(bits, 0, FILAS * COLS);

    for (int r = 0; r < FILAS; r++) {
        for (int c = 0; c < COLS; c++) {
            int srcCol = (c + offset) % textoAncho;
            if (srcCol < 0) srcCol += textoAncho;  // manejar offsets negativos

            // Esta línea asume que textoAncho nunca es 0.
            bits[r][c] = matrizGlobal[r][srcCol];
        }
    }
}

// =========================================================
//              FUNCIÓN DE ESCRITURA DE TEXTO UNIFICADA
// =========================================================

void write_text(const char *text, bool stretch) {
    memset(matrizGlobal, 0, sizeof(matrizGlobal));

    int charWidth = 8;
    int textHeight = stretch ? 16 : 8; // 16 (estirado) o 8 (normal)
    int vOffset = (FILAS - textHeight) / 2;

    int len = strlen(text);
    int maxChars = MAX_COLS / charWidth;
    if (len > maxChars) len = maxChars;

    textoAncho = len * charWidth;  // ancho real en píxeles

    for (int t = 0; t < len; t++) {
        const uint8_t *glyph = getGlyph(text[t]);
        
        for (int row = 0; row < 8; row++) {
            uint8_t bitsRow = glyph[row];
            
            int repeatCount = stretch ? 2 : 1; // Repetir 2 o 1 vez
            
            for (int repeat = 0; repeat < repeatCount; repeat++) {
                int r = row * repeatCount + repeat + vOffset;
                
                if (r < 0 || r >= FILAS) continue;
                
                for (int bit = 0; bit < 8; bit++) {
                    int c = t * charWidth + bit;
                    if (c >= MAX_COLS) continue;
                    
                    matrizGlobal[r][c] = (bitsRow & (1 << (7 - bit))) ? 1 : 0;
                }
            }
        }
    }
}

// =========================================================
//                   FUNCIONES DE RED
// =========================================================

void connectToWiFi() {
    Serial.print("Conectando a ");
    Serial.println(ssid);
    
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Conectado!");
        Serial.print("Dirección IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFALLÓ LA CONEXIÓN WiFi.");
    }
}

void fetchNewQuote() {
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi();
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("No hay conexión WiFi, saltando la actualización.");
            return;
        }
    }

    HTTPClient http;
    http.begin(QUOTE_API_URL);
    int httpCode = http.GET();

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            
            // 1. Limpiar el payload y agregar espacios para el scroll
            String formattedQuote = "   " + payload + "   ";
            
            // 2. Copiar al buffer global, asegurando el límite.
            formattedQuote.toCharArray(currentQuote, MAX_QUOTE_LENGTH);
            
            Serial.print("Nueva Frase: ");
            Serial.println(currentQuote);

            // 3. Re-dibujar la matriz con la nueva frase (false = no estirar)
            write_text(TEXTO_A_MOSTRAR, false); 
            
            // 4. Reiniciar el scroll para la nueva frase
            offset = 0; 
            
        } else {
            Serial.printf("Error en la solicitud HTTP. Código: %d\n", httpCode);
        }
    } else {
        Serial.printf("Fallo en la conexión HTTP: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}

// =========================================================
//                   SETUP Y LOOP
// =========================================================

void inicializar(){
  Serial.begin(115200);
  // Configuración de pines de salida
  pinMode(oe_pin, OUTPUT);
  pinMode(a_pin, OUTPUT);
  pinMode(b_pin, OUTPUT);
  pinMode(c_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(sclk_pin, OUTPUT);
  pinMode(dato_pinR, OUTPUT);

  // Poner pines en estado inicial bajo
  fastWrite(oe_pin, 0);
  fastWrite(a_pin, 0);
  fastWrite(b_pin, 0);
  fastWrite(c_pin, 0);
  fastWrite(sclk_pin, 0);
  fastWrite(clk_pin, 0);
  fastWrite(dato_pinR, 0);
}

void setup() {
  inicializar();
  
  // 1. Conectar a Wi-Fi al inicio
  connectToWiFi();

  // 2. Obtener la primera frase.
  // Si la conexión falla, se mantendrá el texto inicial ("Inicializando...").
  fetchNewQuote(); 

  // 3. Dibujar la matriz
  // Aquí eliges si estirar (true) o no (false) el texto
  write_text(TEXTO_A_MOSTRAR, false); 
  copiar_a_bits(bits, offset);
}

void loop() {
    // === 1. Lógica de Actualización Periódica (no bloqueante) ===
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateMillis >= updateInterval) {
        Serial.println(">>> Tiempo de actualización. Buscando nueva frase...");
        fetchNewQuote();
        lastUpdateMillis = currentMillis;
    }
    
    // === 2. Lógica de Scroll y Dibujo (controla la velocidad del scroll) ===
    if(aux >= refresh){
        copiar_a_bits(bits, offset);
        
        offset++;
        
        // Si offset alcanza el ancho total del texto, reinicia a 0
        if(offset >= textoAncho) {
            offset = 0;
        }
        
        aux = 0;
    }
    aux++;  
    
    // === 3. Dibujar en la matriz ===
    escribe();
}
