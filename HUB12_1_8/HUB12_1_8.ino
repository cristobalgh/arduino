// P4.75 HUB12 1/8 scan 64x16 de un solo color, matrix control by Damián G. Lasso www.LASSO-TECH.com 09/2025
// funciona con la hoja
// https://docs.google.com/spreadsheets/d/1zATPOVYvM4deE6Tu0Ynbwy8HzEikA4DD0I224S6h0Ag/edit?gid=0#gid=0

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "font8x8.h"

#define WIDTH 64
#define HEIGHT 16

 //ESP32
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12

/* //Arduino uno
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      11 // B en conector IDC HUB12
#define clk_pin    10 // S o CLK en conector IDC HUB12
#define sclk_pin   9 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  8 // R o Data en conector IDC HUB12
#define c_pin      7 // C en conector IDC HUB12
*/
uint8_t mensaje[] = {
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010,
0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010, 0b10101010
};

size_t len = sizeof(mensaje) / sizeof(mensaje[0]);
int fila  = 1;
int bit   = 0;
int aux   = 0;
uint8_t matriz[HEIGHT][WIDTH];

void setup() {  
  pinMode(oe_pin, OUTPUT);
  pinMode(a_pin, OUTPUT);
  pinMode(b_pin, OUTPUT);
  pinMode(c_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(sclk_pin, OUTPUT);
  pinMode(dato_pinR, OUTPUT);  
  digitalWrite(oe_pin, 0);
  digitalWrite(a_pin, 0);
  digitalWrite(b_pin, 0);
  digitalWrite(c_pin, 0);
  digitalWrite(sclk_pin, 0);
  digitalWrite(clk_pin, 0);  
  digitalWrite(dato_pinR, 0);  
}

// Convierte texto en matriz 16x64 de 0/1
void texto_a_matriz(const char *texto, uint8_t matriz[HEIGHT][WIDTH]) {
    memset(matriz, 0, HEIGHT * WIDTH);  // limpiar matriz

    int col = 0;
    int row_offset = 0;

    for (int i = 0; texto[i] != '\0'; i++) {
        if (col + 8 > WIDTH) {  
            // si no cabe, saltar a la segunda línea
            col = 0;
            row_offset = 8;
        }
        if (row_offset >= HEIGHT) break; // si ya no cabe más, salir

        unsigned char c = texto[i];
        for (int row = 0; row < 8; row++) {
            uint8_t bits = font8x8_basic[c][row];
            for (int b = 0; b < 8; b++) {
                if (bits & (1 << (7 - b))) {
                    matriz[row_offset + row][col + b] = 1;
                }
            }
        }
        col += 8; // siguiente carácter ocupa 8 columnas
    }
}

void randomizeBits(uint8_t *mensaje, size_t length) {
    for (size_t i = 0; i < length; i++) {
        uint8_t byte = 0;
        for (int bit = 0; bit < 8; bit++) {
            if (rand() % 2) {
                byte |= (1 << bit);
            }
        }
        mensaje[i] = byte;
    }
}

void rotateLeft(uint8_t *mensaje, size_t length) {
    for (size_t i = 0; i < length; i++) {
        uint8_t msb = (mensaje[i] & 0x80) >> 7; // Guardamos el bit más significativo
        mensaje[i] = (mensaje[i] << 1) | msb;   // Shift left y colocamos el MSB como LSB
    }
}
void eligeFila(bool a,bool b,bool c) {
  digitalWrite(a_pin,a);
  digitalWrite(b_pin,b);
  digitalWrite(c_pin,c);
}

void loop() {
  texto_a_matriz("Hola Mundo 123", matriz);

  if(aux >= 5000) {
  randomizeBits(mensaje, len);
  //rotateLeft(mensaje, len);
  aux = 0;
  }
  digitalWrite(oe_pin, 0); // habilita la matriz, enciende los LEDs
  
  if(fila == 1) {eligeFila(1,1,1);} // fila 8 y 16 en la matriz
  if(fila == 2) {eligeFila(0,1,1);}
  if(fila == 3) {eligeFila(1,0,1);}
  if(fila == 4) {eligeFila(0,0,1);}
  if(fila == 5) {eligeFila(1,1,0);}
  if(fila == 6) {eligeFila(0,1,0);}
  if(fila == 7) {eligeFila(1,0,0);}
  if(fila == 0) {eligeFila(0,0,0);}
    
  for (int i = 15 ; i >= 7 ; i--) {
    for (int x = 0 ; x < 64 ; x++ ) {
      if (x == 32) {bit = matriz[i-8][x-32];}
      else {bit = matriz[i][x];}
        digitalWrite(dato_pinR, bit);     // escritura del bit en la salida 
        digitalWrite(clk_pin, 1);         // primer movimiento para consolidar el bit
        digitalWrite(clk_pin, 0);         // segundo movimiento para consolidar el bit 
    }
  }
  /*for(int i=0;i<=120;i+=8) { // for para recorrer byte por byte del arreglo     
      for(int x=7;x>=0;x--) { // for para recorrer bit por bit del byte actual
        bit = bitRead(mensaje[fila+i],x); // lectura del bit dentro del byte actual
        digitalWrite(dato_pinR, bit);     // escritura del bit en la salida 
        digitalWrite(clk_pin, 1);         // primer movimiento para consolidar el bit
        digitalWrite(clk_pin, 0);         // segundo movimiento para consolidar el bit
      //delay(1);
      }
    //delay(100);
  }*/
  
  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  digitalWrite(oe_pin, 1);   // deshabilita la matriz, apaga los LEDs

  fila++; // cambio de fila
  if(fila == 8) fila = 0;
  aux++;   
}
