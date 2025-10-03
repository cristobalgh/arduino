// P4.75 HUB12 1/8 scan 16x64 de un solo color, matrix control.
// ESP32 dev module
#include <letras.h>
#include <WiFi.h>

#include "soc/gpio_struct.h"   // <-- gives you the GPIO struct
#include "driver/gpio.h"       // <-- for gpio_pad_select_gpio, directions

#define oe_pin     13 // OE en conector IDC HUB12
#define a_pin      12 // A en conector IDC HUB12
#define b_pin      14 // B en conector IDC HUB12
#define c_pin      33 // C en conector IDC HUB12
#define clk_pin    27 // S o CLK en conector IDC HUB12
#define sclk_pin   26 // L o SCLK o STB o LAT en conector IDC HUB12
#define dato_pinR  25 // R o Data en conector IDC HUB12

#define FILAS 16
#define COLS  64

int fila  = 1;
int aux   = 0;

char hora[7];
struct tm timeinfo;

// Configura tu WiFi
const char* ssid     = "mosqueton";
const char* password = "esmerilemelo";

// Configuración NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -10800; // UTC-3 Santiago
const int   daylightOffset_sec = 0; // Ajusta si quieres horario de verano

// Inicializa WiFi y NTP
void setupHora() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

//wrapper para funcion dirtecta a registros, mas rapida que
//digitalWrite y digitalWriteFast
inline void fastWrite(uint8_t pin, bool val) {
  if (val) {
    if (pin < 32)
      GPIO.out_w1ts = (1UL << pin);
    else
      GPIO.out1_w1ts.val = (1UL << (pin - 32));
  } else {
    if (pin < 32)
      GPIO.out_w1tc = (1UL << pin);
    else
      GPIO.out1_w1tc.val = (1UL << (pin - 32));
  }
}

// Función que devuelve la hora actual de Santiago como string "HHMMSS"
bool obtenerHora(char* hora_str, size_t len) {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    return false; // Hora no disponible todavía
  }
  strftime(hora_str, len, "%H%M%S", &timeinfo);
  return true;
}

uint8_t bits[FILAS][COLS]; //matriz de trabajo
uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16]; //sub matrices

void subCopiar(){
  copySubMatrix(bits, uno, 8, 0); //copySubMatrix(grande, chica, filaInicial, colInicial)
  copySubMatrix(bits, dos, 0, 0);
  copySubMatrix(bits, tres, 8, 16);
  copySubMatrix(bits, cuatro, 0, 16);
  copySubMatrix(bits, cinco, 8, 32);
  copySubMatrix(bits, seis, 0, 32);
  copySubMatrix(bits, siete, 8, 48);
  copySubMatrix(bits, ocho, 0, 48);
}

void copySubMatrix(uint8_t src[16][64], uint8_t dest[8][16], int startFila, int startCol) {
    for (int fila = 0; fila < 8; fila++) {
        for (int col = 0; col < 16; col++) {
            dest[fila][col] = src[startFila + fila][startCol + col];
        }
    }
}

void rotateVertical(uint8_t bits[FILAS][COLS], int dir) {
    uint8_t temp[COLS];

    if (dir > 0) {
        // Rota hacia abajo
        memcpy(temp, bits[FILAS-1], sizeof(temp));  // guardar última fila
        for (int i = FILAS-1; i > 0; i--) {
            memcpy(bits[i], bits[i-1], sizeof(temp));
        }
        memcpy(bits[0], temp, sizeof(temp));        // poner última arriba
    } else if (dir < 0) {
        // Rota hacia arriba
        memcpy(temp, bits[0], sizeof(temp));        // guardar primera fila
        for (int i = 0; i < FILAS-1; i++) {
            memcpy(bits[i], bits[i+1], sizeof(temp));
        }
        memcpy(bits[FILAS-1], temp, sizeof(temp));  // poner primera abajo
    }
}

void rotate_horizontal(uint8_t matrix[16][64], int direction) {
    if (direction == 1) { // rotar a la derecha
        for (int r = 0; r < 16; r++) {
            uint8_t last = matrix[r][63]; // guardar última columna
            for (int c = 63; c > 0; c--) {
                matrix[r][c] = matrix[r][c-1];
            }
            matrix[r][0] = last; // colocar última en primera
        }
    } else if (direction == -1) { // rotar a la izquierda
        for (int r = 0; r < 16; r++) {
            uint8_t first = matrix[r][0]; // guardar primera columna
            for (int c = 0; c < 63; c++) {
                matrix[r][c] = matrix[r][c+1];
            }
            matrix[r][63] = first; // colocar primera al final
        }
    }
}

void eligeFila(bool a,bool b,bool c) {
  fastWrite(a_pin,a);
  fastWrite(b_pin,b);
  fastWrite(c_pin,c);
}

void escribe(){

//Esta es el orden clave para que se vea bien:
//
//For each row of pixels, we repeat the following cycle of steps:
//
//1. Clock in the data for the current row one bit at a time
//2. Pull the latch and output enable pins high. This enables the latch,
//allowing the row data to reach the output driver but it also disables
//the output so that no LEDs are lit while we're switching rows.
//3. Switch rows by driving the appropriate row select lines.
//4. Pull the latch and output enable pins low again, enabling the
//output and closing the latch so we can clock in the next row of data.

  subCopiar();// matriz bits a 8 chicas
  
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,ocho[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,siete[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,seis[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }    
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,cinco[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,cuatro[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,tres[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,dos[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }
  for(int j=15;j>=0;j--){
    fastWrite(dato_pinR,uno[fila-1][j]);
    fastWrite(clk_pin,1);
    fastWrite(clk_pin,0);
  }
  
  fastWrite(oe_pin, 1);   // output disable
  fastWrite(sclk_pin, 1); // primer movimiento latch

  if(fila == 1) {eligeFila(1,1,1);}
  if(fila == 2) {eligeFila(0,1,1);}
  if(fila == 3) {eligeFila(1,0,1);}
  if(fila == 4) {eligeFila(0,0,1);}
  if(fila == 5) {eligeFila(1,1,0);}
  if(fila == 6) {eligeFila(0,1,0);}
  if(fila == 7) {eligeFila(1,0,0);}
  if(fila == 8) {eligeFila(0,0,0);}

  fastWrite(sclk_pin, 0); // segundo movimiento latch
  fastWrite(oe_pin, 0);   // output enable

  fila++; // cambio de fila
  if(fila == 9) fila = 1;
}

void setup() {  
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
  setupHora();//se conecta al wifi y recibe hora local
  while (!getLocalTime(&timeinfo)) {
    delay(1000);
  }
}

void loop() {  
//  if(aux >= 300){
//    rotate_horizontal(bits,1);
//    rotateVertical(bits, 1);
//    aux = 0;
//  }
//  aux++;
 
  obtenerHora(hora,sizeof(hora));//hora actual ya cargada
  write_text(bits,hora);
  escribe();
}
