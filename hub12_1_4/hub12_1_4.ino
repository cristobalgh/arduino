// P10 HUB12 1/4 scan 32x16  de dos colores (R y G), matrix control by Damián G. Lasso www.LASSO-TECH.com 07/2024

//#define oe_pin     13  // OE en conector IDC HUB12
//#define a_pin      12  // A en conector IDC HUB12
//#define b_pin      14  // B en conector IDC HUB12
//#define clk_pin    27  // S o CLK en conector IDC HUB12
//#define sclk_pin   26  // L o SCLK o STB o LAT en conector IDC HUB12
//#define dato_pinR  25  // R o DATAR o DATA1 en conector IDC HUB12
//#define dato_pinG  33  // G o DATAG o DATA2 en conector IDC HUB12

#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12

int fila = 3;

void setup() 
{  
//  pinMode(oe_pin, OUTPUT);
//  pinMode(a_pin, OUTPUT);
//  pinMode(b_pin, OUTPUT);
//  pinMode(clk_pin, OUTPUT);
//  pinMode(sclk_pin, OUTPUT);
//  pinMode(dato_pinR, OUTPUT);
//  pinMode(dato_pinG, OUTPUT);

  pinMode(oe_pin, OUTPUT);
  pinMode(a_pin, OUTPUT);
  pinMode(b_pin, OUTPUT);
  pinMode(c_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(sclk_pin, OUTPUT);
  pinMode(dato_pinR, OUTPUT);  
 // pinMode(dato_pinG, OUTPUT);

  
  digitalWrite(oe_pin, 0);
  digitalWrite(a_pin, 0);
  digitalWrite(b_pin, 0);
  digitalWrite(c_pin, 0);
  digitalWrite(sclk_pin, 0);
  digitalWrite(clk_pin, 0);  
}

void loop() 
{
  for(int x=0;x<8;x++)
  {
  // Determina el valor a escribir:
    // Si 'x' es par (0, 2, 4, 6), escribe 1.
    // Si 'x' es impar (1, 3, 5, 7), escribe 0.
    int valor = (x % 2 == 0) ? 1 : 0;
    
    // Escribe el valor alternado al pin.
    digitalWrite(dato_pinR, valor);  // Escritura del bit alternado
    
    // Los pulsos de reloj (CLK) se mantienen iguales.
    digitalWrite(clk_pin, 1); // primer movimiento para consolidar el bit
    digitalWrite(clk_pin, 0); // segundo movimiento para consolidar el bit
  }

  digitalWrite(oe_pin, 0); // deshabilita la matriz, apaga los LEDs
  digitalWrite(sclk_pin, 1); // primer movimiento para consolidar el byte

//  if(fila == 1) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 0); digitalWrite(c_pin, 0); } // Selectores de fila
//  if(fila == 2) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 0); digitalWrite(c_pin, 1); }
//  if(fila == 3) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 1); digitalWrite(c_pin, 0); }
//  if(fila == 4) { digitalWrite(a_pin, 0); digitalWrite(b_pin, 1); digitalWrite(c_pin, 1); }
//  if(fila == 5) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 0); digitalWrite(c_pin, 0); }
//  if(fila == 6) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 0); digitalWrite(c_pin, 1); }
//  if(fila == 7) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 1); digitalWrite(c_pin, 0); }
//  if(fila == 8) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 1); digitalWrite(c_pin, 1); }

  if(fila == 1) { digitalWrite(a_pin, 0); digitalWrite(c_pin, 0); } //nada
  if(fila == 2) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 1); } //algo
  if(fila == 3) { digitalWrite(a_pin, 0); digitalWrite(c_pin, 1); } //nada
  if(fila == 4) { digitalWrite(a_pin, 1); digitalWrite(b_pin, 0); } //algo
  
  digitalWrite(sclk_pin, 0); // segundo movimiento para consolidar el byte
  digitalWrite(oe_pin, 1); // habilita la matriz, enciende los LEDs
  //delay(500);
  
  //fila++; // cambio de fila
  //if(fila > 4) fila = 1; 
}
