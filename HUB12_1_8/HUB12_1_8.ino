// P4.75 HUB12 1/8 scan 64x16 de un solo color, matrix control by Damián G. Lasso www.LASSO-TECH.com 09/2025
/* //ESP32
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12
*/
//Arduino1
#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      11 // B en conector IDC HUB12
#define clk_pin    10 // S o CLK en conector IDC HUB12
#define sclk_pin   9 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  8 // R o Data en conector IDC HUB12
#define c_pin      7 // C en conector IDC HUB12

uint8_t mensaje[] = {0b00000000, 0b00000001, 0b00011010, 0b00100000, 0b01100000, 0b00100000, 0b00100000, 0b00010000, 0b00000000, 0b10101110, 0b00000001, 0b00000001, 0b00000000, 0b00000000, 0b00000000, 0b00001111, 0b00001100, 0b00000011, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10110110, 0b00000000, 0b00011000, 0b10010000, 0b01010101, 0b00010000, 0b00011000, 0b00111000, 0b11000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10000000, 0b01100000, 0b00001111, 0b00000000, 0b11111110,  0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11101010, 0b00000000, 0b00000001, 0b00000011, 0b00000001, 0b01000000, 0b00010100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b10000010, 0b11000110, 0b01111100, 0b11000000, 0b00110000, 0b00011111, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10111111, 0b01100000, 0b10000000, 0b10000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b10111111, 0b00000001, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000001, 0b00000000, 0b00000010, 0b00000001, 0b01010011, 0b00000000,0b01111110, 0b10000001, 0b00000001, 0b00000000, 0b00000000, 0b00000000, 0b11000000, 0b00111110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11100000, 0b10010000, 0b10000100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000010, 0b10000001, 0b00000000, 0b10000000, 0b00000000, 0b00000000, 0b00111111, 0b11000000, 0b00000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b10000000, 0b00000000, 0b00000000};

int fila = 0;
int bit = 0;

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

void loop() 
{
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
    }    
  }

  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  digitalWrite(oe_pin, 1);   // deshabilita la matriz, apaga los LEDs
  //delay(1); // atenuación, opcional

  fila++; // cambio de fila
  if(fila > 7) fila = 0;   
}
