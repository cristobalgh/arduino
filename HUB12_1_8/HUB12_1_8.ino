// P4.75 HUB12 1/8 scan 64x16 de un solo color, matrix control by Damián G. Lasso www.LASSO-TECH.com 09/2025
// funciona con la hoja
// https://docs.google.com/spreadsheets/d/1zATPOVYvM4deE6Tu0Ynbwy8HzEikA4DD0I224S6h0Ag/edit?gid=0#gid=0

#include <time.h>

#include <Arduino.h>

 //ESP32
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12

////Arduino uno
//#define oe_pin     13 // OE en conector IDC HUB12
//#define a_pin      12 // A en conector IDC HUB12
//#define b_pin      11 // B en conector IDC HUB12
//#define clk_pin    10 // S o CLK en conector IDC HUB12
//#define sclk_pin   9 // L o SCLK o STB o LAT en conector IDC HUB12
//#define dato_pinR  8 // R o Data en conector IDC HUB12
//#define c_pin      7 // C en conector IDC HUB12

//const char *cadena = "1111011100000110010010100100010000100100110011110111001001001110100010011100111110100010000010000010100010101000001110000000000010000000100010010100101001000100001010010010100001001010010000001000101000101000001000100000100000101000101010000100010000010000111000110000100101011011110001110011100100101110011100111100000010001010001010000010001000001000001010001010100001000100010101001000010000001001011010100100010010100101001010000100101001000000111110100010100000111110000010010010111110101000010001000011100011110011100010010100100110000111001110010010111101110001100000001000101000101000001000100000100100101000101011100100010011111110000000000000000000000000000000000000000000000000000000000000000010001010001010000010001000001010101010001010100101000100001110001111111111111111111111111111111111111111111111111111111111111111100010100010100000010100000011000110010100101001010001000101010000000000000000000000000000000000000000000000000000000000000000001000100111001000000010000000100000100010001011100011100000010000";
#define BITS_LEN(strlen) (((strlen) + 7) / 8)
size_t n = BITS_LEN(16);  // en este caso 16 bits
//uint8_t mensaje[BITS_LEN(16)];

/*
uint8_t mensaje[] = { //lineas verticales
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
*/

uint8_t mensaje[] = { //hola maipo
0b11110111, 0b10000000, 0b11100011, 0b10000100, 0b11110011, 0b00000000, 0b11111111, 0b00000000,
0b00000110, 0b10001001, 0b00001001, 0b00001001, 0b10001001, 0b00000000, 0b11111111, 0b00000000,
0b10001001, 0b10001010, 0b10001010, 0b11111010, 0b10001010, 0b10001010, 0b10001010, 0b10001001,
0b11001111, 0b00101000, 0b00101000, 0b00101000, 0b00101000, 0b00101000, 0b00101000, 0b11001000,
0b01001010, 0b01001010, 0b01011011, 0b01101010, 0b01001001, 0b00000000, 0b11111111, 0b00000000,
0b01000100, 0b01000100, 0b11000111, 0b01000100, 0b10000111, 0b00000000, 0b11111111, 0b00000000,
0b10100010, 0b00100010, 0b00100010, 0b00111110, 0b00100010, 0b00100010, 0b00010100, 0b00001000,
0b00001000, 0b00001000, 0b00001000, 0b00001001, 0b00001001, 0b00001010, 0b00001100, 0b00001000,
0b00100100, 0b00101001, 0b00111001, 0b10100101, 0b00111001, 0b00000000, 0b11111111, 0b00000000,
0b11001111, 0b00101000, 0b00101110, 0b00101000, 0b00101111, 0b00000000, 0b11111111, 0b00000000,
0b00101000, 0b00101000, 0b00101000, 0b00101111, 0b00101000, 0b10101000, 0b01100101, 0b00100010,
0b10101000, 0b10101000, 0b10101000, 0b10101000, 0b10101110, 0b10101001, 0b00101001, 0b00101110,
0b01110010, 0b01001010, 0b01110011, 0b01001010, 0b01110001, 0b00000000, 0b11111111, 0b00000000,
0b01001110, 0b01000000, 0b11000000, 0b01000000, 0b10000000, 0b00000000, 0b11111111, 0b00000000,
0b00111000, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b01000100, 0b00111000,
0b00000000, 0b00010000, 0b01010100, 0b00111000, 0b11111110, 0b00111000, 0b01010100, 0b00010000                                                                                                                                              
};

void stringToBytes(const char *bits, uint8_t *out, size_t out_len) {
    size_t i, byteIndex = 0, bitCount = 0;
    uint8_t byte = 0;

    for (i = 0; bits[i] != '\0'; i++) {
        byte = (byte << 1) | (bits[i] - '0');
        bitCount++;

        if (bitCount == 8) {
            out[byteIndex++] = byte;
            byte = 0;
            bitCount = 0;
        }
    }

    if (bitCount > 0 && byteIndex < out_len) {
        byte <<= (8 - bitCount);
        out[byteIndex++] = byte;
    }
}

size_t len = sizeof(mensaje) / sizeof(mensaje[0]);
int fila  = 1;
int bit   = 0;
int aux   = 0;

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
  //stringToBytes(cadena, mensaje, n);
}

void loop() {
  if(aux >= 5000) {
  //randomizeBits(mensaje, len);
  rotateLeft(mensaje, len);
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
    
  /*for (int i = 15 ; i >= 7 ; i--) { //para recorrer otra forma
    for (int x = 0 ; x < 64 ; x++ ) {
      if (x == 32) {bit = matriz2[i-8][x-32];}
      else {bit = matriz2[i][x];}
        digitalWrite(dato_pinR, bit);     // escritura del bit en la salida 
        digitalWrite(clk_pin, 1);         // primer movimiento para consolidar el bit
        digitalWrite(clk_pin, 0);         // segundo movimiento para consolidar el bit 
    }
  }*/
  for(int i=0;i<=120;i+=8) { // for para recorrer byte por byte del arreglo     
      for(int x=7;x>=0;x--) { // for para recorrer bit por bit del byte actual
        bit = bitRead(mensaje[fila+i],x); // lectura del bit dentro del byte actual
        digitalWrite(dato_pinR, bit);     // escritura del bit en la salida 
        digitalWrite(clk_pin, 1);         // primer movimiento para consolidar el bit
        digitalWrite(clk_pin, 0);         // segundo movimiento para consolidar el bit
      //delay(1);
      }
    //delay(100);
  }
  
  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  digitalWrite(oe_pin, 1);   // deshabilita la matriz, apaga los LEDs

  fila++; // cambio de fila
  if(fila == 8) fila = 0;
  aux++;   
}
