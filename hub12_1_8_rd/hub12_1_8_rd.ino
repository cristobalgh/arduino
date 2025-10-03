// P4.75 HUB12 1/8 scan 16x64 de un solo color, matrix control.
// ESP32 dev module

#include <letras.h>
#include <WiFi.h>
#include "soc/gpio_struct.h"   // acceso directo a registros GPIO

#define oe_pin     13
#define a_pin      12
#define b_pin      14
#define c_pin      33
#define clk_pin    27
#define sclk_pin   26
#define dato_pinR  25

#define FILAS 16
#define COLS  64

int fila  = 1;
int aux   = 0;

char hora[7];
struct tm timeinfo; 

// WiFi
const char* ssid     = "mosqueton";
const char* password = "esmerilemelo";

// NTP y zona horaria Chile
const char* ntpServer = "pool.ntp.org";
const char* tzInfo    = "CLT3CLST,M10.2.0/0,M3.2.0/0"; 

// resincronización cada hora
const unsigned long RESYNC_INTERVAL = 3600000;  
unsigned long lastNtpSync = 0;

// ---------- Fast GPIO -----------
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

// ---------- Hora -----------
void setupHora() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  configTime(0, 0, ntpServer);
  setenv("TZ", tzInfo, 1);  // zona horaria Chile
  tzset();
}

void reSyncHora() {
  if (WiFi.status() == WL_CONNECTED) {
    configTime(0, 0, ntpServer);
    setenv("TZ", tzInfo, 1);
    tzset();
    Serial.println("[NTP] Resincronización solicitada");
  }
}

bool obtenerHora(char* hora_str, size_t len) {
  if (!getLocalTime(&timeinfo, 100)) { // timeout 100ms
    return false;
  }
  strftime(hora_str, len, "%H%M%S", &timeinfo);
  return true;
}

// ---------- Matriz buffers -----------
uint8_t bits[FILAS][COLS];
uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16];

void copySubMatrix(uint8_t src[16][64], uint8_t dest[8][16], int startFila, int startCol) {
  for (int f = 0; f < 8; f++) {
    for (int c = 0; c < 16; c++) {
      dest[f][c] = src[startFila + f][startCol + c];
    }
  }
}

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

// ---------- Setup y Loop -----------
void setup() {
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

  setupHora();

  while (!getLocalTime(&timeinfo)) {
    delay(1000);
  }
  Serial.println("[NTP] Hora inicial sincronizada");
}

void loop() {
  if (obtenerHora(hora, sizeof(hora))) {
    write_text(bits, hora);
  }
  escribe();

  unsigned long now = millis();
  if (now - lastNtpSync > RESYNC_INTERVAL) {
    reSyncHora();
    lastNtpSync = now;
  }

  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
  }
}
