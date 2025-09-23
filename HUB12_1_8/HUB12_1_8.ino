// P4.75 HUB12 1/8 scan 64x16 de un solo color, matrix control by Damián G. Lasso www.LASSO-TECH.com 09/2025
// funciona con la hoja
// https://docs.google.com/spreadsheets/d/1zATPOVYvM4deE6Tu0Ynbwy8HzEikA4DD0I224S6h0Ag/edit?gid=0#gid=0

#include <time.h>
/* //ESP32
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12
*/
//Arduino uno
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      11 // B en conector IDC HUB12
#define clk_pin    10 // S o CLK en conector IDC HUB12
#define sclk_pin   9 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  8 // R o Data en conector IDC HUB12
#define c_pin      7 // C en conector IDC HUB12

uint8_t mensaje[] = {
0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00000100, 0b00001001, 0b00010010, 0b00100100, 0b01001001, 0b10110110, 0b00110110, 0b01001001, 0b10010100, 0b00100010, 0b01000001, 0b10000000, 0b01001001, 0b10010010, 0b00100100, 0b01001000, 0b10010000, 0b00100000, 0b01000000, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b01000000, 0b00100000, 0b10010000, 0b01001000, 0b00100100, 0b10010010,  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01001001, 0b00100100, 0b00010010, 0b00001001, 0b00000100, 0b00000010, 0b00000001, 0b00000000, 0b00000001, 0b10000010, 0b01000100, 0b00101001, 0b10010010, 0b01101100, 0b01101101, 0b10010010, 0b00000001, 0b00000010, 0b00000100, 0b00001001, 0b00010010, 0b00100100, 0b01001001, 0b10010010, 0b00100100, 0b11011010, 0b11011001, 0b00100100, 0b01010010, 0b10001001, 0b00000100, 0b00000010, 0b00100100, 0b01001000, 0b10010000, 0b00100000, 0b01000000, 0b10000000, 0b00000000, 0b00000000,0b00000001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b01000000, 0b00100000, 0b10010000, 0b01001000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b00000010, 0b00100100, 0b10010010, 0b01001001, 0b00100100, 0b00010010, 0b00001001, 0b00000101, 0b00000010, 0b00000100, 0b00001001, 0b0001001, 0b1010010, 0b01001001, 0b10110010, 0b10110100, 0b01001000};

size_t len = sizeof(mensaje) / sizeof(mensaje[0]);

int fila = 0;
int bit = 0;
int aux = 0;

void setup() 
{  
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

void loop() 
{

 if(aux >=5000){
  randomizeBits(mensaje, len);
  //rotateLeft(mensaje, len);
  aux = 0;
  }
  digitalWrite(oe_pin, 0); // habilita la matriz, enciende los LEDs

  if(fila == 0) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 0); digitalWrite(c_pin, 0); } // Selectores de fila
  if(fila == 1) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 1); digitalWrite(c_pin, 1); }
  if(fila == 2) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 1); digitalWrite(c_pin, 1); }
  if(fila == 3) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 0); digitalWrite(c_pin, 1); }
  if(fila == 4) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 0); digitalWrite(c_pin, 1); }
  if(fila == 5) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 1); digitalWrite(c_pin, 0); }
  if(fila == 6) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 1); digitalWrite(c_pin, 0); }  
  if(fila == 7) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 0); digitalWrite(c_pin, 0); } 
  
  for(int i=0;i<=120;i+=8) // for para recorrer byte por byte del arreglo
  {     
    for(int x=7;x>=0;x--) // for para recorrer bit por bit del byte actual
    {
      bit = bitRead(mensaje[fila+i],x); // lectura del bit dentro del byte actual
      digitalWrite(dato_pinR, bit);     // escritura del bit en la salida 
      digitalWrite(clk_pin, 1);         // primer movimiento para consolidar el bit
      digitalWrite(clk_pin, 0);         // segundo movimiento para consolidar el bit
      //delay(1);
    }
    //delay(5);    
  }
  
  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  digitalWrite(oe_pin, 1);   // deshabilita la matriz, apaga los LEDs
  //delay(1); // atenuación, opcional

  fila++; // cambio de fila
  if(fila > 7) fila = 0;
  aux++;   
}
