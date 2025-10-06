// P4.75 HUB12 1/8 scan 16x64 de un solo color, matrix control.
// scroll text maximo MAX_COLS, caracteres de 8 bits de ancho
// ESP32 dev module

#include "soc/gpio_struct.h"
#include "letras.h"

#define oe_pin     13
#define a_pin      12
#define b_pin      14
#define c_pin      33
#define clk_pin    27
#define sclk_pin   26
#define dato_pinR  25
#define FILAS 16
#define COLS  64

// ===== Variable Global para guardar el texto a mostrar =====
//const char *TEXTO_A_MOSTRAR = "   La imaginaci}n es m{s importante que el conocimiento.   A.E.    ";
const char *TEXTO_A_MOSTRAR = "   Ich sorge mich nie um die Zukunft. Sie kommt fr[h genug.   ";

#define MAX_COLS COLS*10   // hasta 80 caracteres de 8x8 

uint8_t matrizGlobal[16][MAX_COLS];  // matriz global de 16x320 o 40 caracteresde 8x8
int textoAncho = 0;  // ancho real del texto dibujado en píxeles
int fila  = 1;
int refresh = 300;  // velocidad de scroll, menor es mas rapido
int aux   = 0;
int offset  = 0;
uint8_t bits[FILAS][COLS];

uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16];

void subCopiar() {
  copySubMatrix(bits, uno,   8,  0);
  copySubMatrix(bits, dos,   0,  0);
  copySubMatrix(bits, tres,  8, 16);
  copySubMatrix(bits, cuatro,0, 16);
  copySubMatrix(bits, cinco, 8, 32);
  copySubMatrix(bits, seis,  0, 32);
  copySubMatrix(bits, siete, 8, 48);
  copySubMatrix(bits, ocho,  0, 48);
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
    case 1: eligeFila(1,1,1); break;
    case 2: eligeFila(0,1,1); break;
    case 3: eligeFila(1,0,1); break;
    case 4: eligeFila(0,0,1); break;
    case 5: eligeFila(1,1,0); break;
    case 6: eligeFila(0,1,0); break;
    case 7: eligeFila(1,0,0); break;
    case 8: eligeFila(0,0,0); break;
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

            bits[r][c] = matrizGlobal[r][srcCol];
        }
    }
}

////esta estira los caracteres de 8x8 a 16x8
//void write_text(const char *text) {
//    memset(matrizGlobal, 0, sizeof(matrizGlobal));
//
//    int charWidth = 8;
//    int textHeight = 16;
//    int vOffset = (FILAS - textHeight) / 2;
//
//    int len = strlen(text);
//    int maxChars = MAX_COLS / charWidth;
//    if (len > maxChars) len = maxChars;
//
//    textoAncho = len * charWidth; // ancho real en píxeles
//
//    for (int t = 0; t < len; t++) {
//        const uint8_t *glyph = getGlyph8(text[t]);
//        for (int row = 0; row < 8; row++) {
//            uint8_t bitsRow = glyph[row];
//            for (int repeat = 0; repeat < 2; repeat++) {
//                int r = row * 2 + repeat + vOffset;
//                if (r < 0 || r >= FILAS) continue;
//                for (int bit = 0; bit < 8; bit++) {
//                    int c = t * charWidth + bit;
//                    if (c >= MAX_COLS) continue;
//                    matrizGlobal[r][c] = (bitsRow & (1 << (7 - bit))) ? 1 : 0;
//                }
//            }
//        }
//    }
//}

//esta no los estira
void write_text(const char *text) {
    memset(matrizGlobal, 0, sizeof(matrizGlobal));

    int charWidth = 8;
    // 1. Reducir la altura del texto a 8 (el tamaño real del glifo)
    int textHeight = 8; 
    
    // 2. Recalcular el desplazamiento vertical para centrar el bloque de texto de 8 filas
    int vOffset = (FILAS - textHeight) / 2; 

    int len = strlen(text);
    int maxChars = MAX_COLS / charWidth;
    if (len > maxChars) len = maxChars;

    textoAncho = len * charWidth; // ancho real en píxeles

    for (int t = 0; t < len; t++) {
        const uint8_t *glyph = getGlyph8(text[t]);
        for (int row = 0; row < textHeight; row++) { // textHeight es 8
            uint8_t bitsRow = glyph[row];
            
            // 3. Eliminar el bucle 'repeat' y dibujar solo una vez por fila del glifo
            int r = row + vOffset;
            
            if (r < 0 || r >= FILAS) continue;
            
            for (int bit = 0; bit < 8; bit++) {
                int c = t * charWidth + bit;
                if (c >= MAX_COLS) continue;
                
                // El bit se dibuja en la posición (r, c)
                matrizGlobal[r][c] = (bitsRow & (1 << (7 - bit))) ? 1 : 0;
            }
        }
    }
}
void inicializar(){
  Serial.begin(115200);
  pinMode(oe_pin, OUTPUT);
  pinMode(a_pin, OUTPUT);
  pinMode(b_pin, OUTPUT);
  pinMode(c_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(sclk_pin, OUTPUT);
  pinMode(dato_pinR, OUTPUT);

  fastWrite(oe_pin, 0);
  fastWrite(a_pin, 0);
  fastWrite(b_pin, 0);
  fastWrite(c_pin, 0);
  fastWrite(sclk_pin, 0);
  fastWrite(clk_pin, 0);
  fastWrite(dato_pinR, 0);
  }

// ===== Setup y Loop =====
void setup() {
  inicializar();
  write_text(TEXTO_A_MOSTRAR);
  copiar_a_bits(bits,offset);
}

void loop() {
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
  escribe();
}
