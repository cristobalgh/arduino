// P4.75 HUB12 1/8 scan 16x64 de un solo color, matrix control.
// scroll text maximo MAX_COLS, caracteres de 8 bits de ancho
// portal captivo para ingresar wifi

#include "soc/gpio_struct.h"
#include <WiFi.h>           // Para conexión Wi-Fi
#include <WebServer.h>      // Para el servidor web de configuración
#include <DNSServer.h>      // Para el portal cautivo
#include <HTTPClient.h>     // Para hacer solicitudes HTTP (descargar frases)
#include "letras.h"         // Se asume que este archivo contiene getGlyph() y la font data

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

// ===== CONSTANTES DE RED Y SERVIDOR =====
WebServer server(80);
DNSServer dnsServer;
const char* AP_SSID = "mLed";

// Buffer y constantes para almacenar la frase descargada
#define MAX_QUOTE_LENGTH 440
char currentQuote[MAX_QUOTE_LENGTH] = "Iniciando portal de configuracion...";

// URL de una API de frases
const char* QUOTE_API_URL = "http://api.forismatic.com/api/1.0/?method=getQuote&lang=en&format=text";

// Control de tiempo para la actualización (10 minutos en milisegundos)
unsigned long lastUpdateMillis = 0;
const long updateInterval = 10UL * 60UL * 1000UL;

// ===== VARIABLES GLOBALES DE SCROLL Y MATRIZ =====
#define MAX_CHARS_SUPPORTED 400
#define MAX_COLS (MAX_CHARS_SUPPORTED * 8)

uint8_t matrizGlobal[FILAS][MAX_COLS];
int textoAncho = 0;
int fila = 1;
//int refresh = 310;

// int refresh = 310; // <-- REEMPLAZA ESTA LÍNEA
const int REFRESH_NORMAL = 310;   // Velocidad para el modo normal
const int REFRESH_CONFIG = 25;    // Velocidad MUCHO MÁS RÁPIDA para el portal
int refresh = REFRESH_NORMAL;     // La variable que usaremos

int aux = 0;
int offset = 0;
uint8_t bits[FILAS][COLS];

uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16];

const char *TEXTO_A_MOSTRAR = currentQuote;

// ===== ESTADOS DE CONECTIVIDAD =====
enum WiFiState {
    WF_DISCONNECTED,
    WF_CONFIG_MODE,
    WF_CONNECTING,
    WF_CONNECTED
};
WiFiState wifiState = WF_DISCONNECTED;

char current_ssid[33] = "";
char current_password[65] = "";

#define FONT_LEN (sizeof(font8x8)/sizeof(font8x8[0]))

// Busca el glifo de un caracter
const uint8_t* getGlyph(char ch) {
    for (int i = 0; i < FONT_LEN; i++) {
        if (font8x8[i].ch == ch) return font8x8[i].glyph;
    }
    return font8x8[FONT_LEN-1].glyph; // si no existe, retorna espacio
}

void copySubMatrix(uint8_t src[16][64], uint8_t dest[8][16], int startFila, int startCol) {
  for (int f = 0; f < 8; f++) {
    for (int c = 0; c < 16; c++) {
      dest[f][c] = src[startFila + f][startCol + c];
    }
  }
}

// =========================================================
//                  FUNCIONES DE GRÁFICOS Y HW
// =========================================================

void subCopiar() {
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
            if (srcCol < 0) srcCol += textoAncho;
            if (textoAncho > 0) {
              bits[r][c] = matrizGlobal[r][srcCol];
            }
        }
    }
}

// =========================================================
//           FUNCIÓN DE ESCRITURA DE TEXTO
// =========================================================

void write_text(const char *text, bool stretch) {
    memset(matrizGlobal, 0, sizeof(matrizGlobal));
    int charWidth = 8;
    int textHeight = stretch ? 16 : 8;
    int vOffset = (FILAS - textHeight) / 2;
    int len = strlen(text);
    if (len > MAX_CHARS_SUPPORTED) len = MAX_CHARS_SUPPORTED;

    textoAncho = len * charWidth;

    for (int t = 0; t < len; t++) {
        const uint8_t *glyph = getGlyph(text[t]);
        for (int row = 0; row < 8; row++) {
            uint8_t bitsRow = glyph[row];
            int repeatCount = stretch ? 2 : 1;
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
//            FUNCIONES DEL PORTAL DE CONFIGURACIÓN
// =========================================================

void handleRoot() {
    Serial.println("Cliente conectado al portal.");
    write_text(" Cliente conectado. Escaneando redes... ", false);
    offset = 0;

    String page = "<!DOCTYPE html><html><head><title>Configuracion WiFi</title>";
    page += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    page += "<style>body{background-color:#333;color:#fff;font-family:sans-serif;text-align:center;margin-top:50px;}";
    page += "h1{color:#00aaff;}";
    page += "select,input,button{width:80%;max-width:300px;padding:12px;margin:8px;border:1px solid #555;border-radius:5px;background-color:#444;color:#fff;font-size:16px;}";
    page += "button{background-color:#007bff;cursor:pointer;}</style></head><body>";
    page += "<h1>Configurar WiFi para Matriz LED</h1>";
    page += "<p>Seleccione una red e ingrese la contrasena.</p>";

    int n = WiFi.scanNetworks();
    if (n == 0) {
        page += "<p>No se encontraron redes. Refresque para reintentar.</p>";
    } else {
        page += "<form action='/connect' method='POST'>";
        page += "<select name='ssid'>";
        for (int i = 0; i < n; ++i) {
            page += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + " (" + WiFi.RSSI(i) + "dBm)</option>";
        }
        page += "</select><br>";
        page += "<input type='password' name='password' placeholder='Contrasena'><br>";
        page += "<button type='submit'>Conectar</button>";
        page += "</form>";
    }
    
    page += "</body></html>";
    server.send(200, "text/html", page);
}

void handleConnect() {
    Serial.println("Recibidas credenciales WiFi.");
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    ssid.toCharArray(current_ssid, sizeof(current_ssid));
    password.toCharArray(current_password, sizeof(current_password));

    String responsePage = "<!DOCTYPE html><html><head><title>Conectando...</title>";
    responsePage += "<meta http-equiv='refresh' content='5; url=/' />";
    responsePage += "<style>body{background-color:#333;color:#fff;font-family:sans-serif;text-align:center;margin-top:50px;}</style></head><body>";
    responsePage += "<h1>Intentando conectar a '" + ssid + "'...</h1>";
    responsePage += "<p>Si la conexion es exitosa, el dispositivo comenzara a mostrar frases. De lo contrario, el portal se recargara.</p>";
    responsePage += "</body></html>";
    server.send(200, "text/html", responsePage);
    
    delay(1000);

    wifiState = WF_CONNECTING;
    char connectMsg[100];
    sprintf(connectMsg, " Conectando a %s... ", current_ssid);
    write_text(connectMsg, false);
    offset = 0;
}

void startConfigPortal() {
    Serial.println("Iniciando portal de configuracion (Access Point).");
    char portalMsg[100];
    sprintf(portalMsg, " Conectese a la red: %s ", AP_SSID);
    write_text(portalMsg, false);
    offset = 0;

    WiFi.softAP(AP_SSID);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    dnsServer.start(53, "*", myIP);

    server.on("/", HTTP_GET, handleRoot);
    server.on("/connect", HTTP_POST, handleConnect);
    server.onNotFound(handleRoot);
    server.begin();

    Serial.println("Servidor HTTP iniciado.");
}

// =========================================================
//                  FUNCIONES DE RED
// =========================================================

bool connectToWiFi(const char* ssid, const char* password) {
    Serial.print("Intentando conectar a: ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 30) {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Conectado!");
        Serial.print("Direccion IP: ");
        Serial.println(WiFi.localIP());
        return true;
    } else {
        Serial.println("\nFALLO LA CONEXION WiFi.");
        WiFi.disconnect();
        return false;
    }
}

void fetchNewQuote() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("No hay conexion WiFi, saltando la actualizacion.");
        wifiState = WF_DISCONNECTED; // Si se pierde la conexión, volver al portal
        return;
    }

    HTTPClient http;
    http.begin(QUOTE_API_URL);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        String formattedQuote = "   " + payload + "   ";
        formattedQuote.toCharArray(currentQuote, MAX_QUOTE_LENGTH);
        
        Serial.print("Nueva Frase: ");
        Serial.println(currentQuote);

        write_text(TEXTO_A_MOSTRAR, false);
        offset = 0;
    } else {
        Serial.printf("Error en la solicitud HTTP. Codigo: %d\n", httpCode);
    }
    http.end();
}

// =========================================================
//                     SETUP Y LOOP
// =========================================================

void inicializar(){
  Serial.begin(115200);
  pinMode(oe_pin, OUTPUT);
  pinMode(a_pin, OUTPUT);
  pinMode(b_pin, OUTPUT);
  pinMode(c_pin, OUTPUT);
  pinMode(clk_pin, OUTPUT);
  pinMode(sclk_pin, OUTPUT);
  pinMode(dato_pinR, OUTPUT);

  fastWrite(oe_pin, 1);
  fastWrite(a_pin, 0);
  fastWrite(b_pin, 0);
  fastWrite(c_pin, 0);
  fastWrite(sclk_pin, 0);
  fastWrite(clk_pin, 0);
  fastWrite(dato_pinR, 0);
}

void setup() {
  inicializar();
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  write_text(TEXTO_A_MOSTRAR, false);
  copiar_a_bits(bits, offset);
}

void loop() {
    // Esta función debe ejecutarse siempre para mantener la matriz refrescada
    escribe();

    // Máquina de estados para manejar la conectividad
    switch (wifiState) {
        case WF_DISCONNECTED:
            refresh = REFRESH_CONFIG; // <-- CAMBIO: Usar velocidad rápida para el portal
            startConfigPortal();
            wifiState = WF_CONFIG_MODE;
            break;

        case WF_CONFIG_MODE:
            dnsServer.processNextRequest();
            server.handleClient();
            // Lógica de scroll para el mensaje de configuración
            if(aux >= refresh){
                copiar_a_bits(bits, offset);
                offset++;
                if(offset >= textoAncho) { offset = 0; }
                aux = 0;
            }
            aux++;
            break;

        case WF_CONNECTING:
            if (connectToWiFi(current_ssid, current_password)) {
                wifiState = WF_CONNECTED;
                refresh = REFRESH_NORMAL; // <-- CAMBIO: Volver a la velocidad normal
                server.stop();
                dnsServer.stop();
                Serial.println("Conexion exitosa! Pasando a modo normal.");
                fetchNewQuote();
                lastUpdateMillis = millis();
            } else {
                Serial.println("Fallo la conexion. Volviendo al modo de configuracion.");
                char errorMsg[100];
                sprintf(errorMsg, " Fallo la conexion a %s. Reiniciando portal... ", current_ssid);
                write_text(errorMsg, false);
                offset = 0;
                delay(5000);
                wifiState = WF_DISCONNECTED;
            }
            break;

        case WF_CONNECTED:
            // Lógica original del programa
            unsigned long currentMillis = millis();
            if (currentMillis - lastUpdateMillis >= updateInterval) {
                Serial.println(">>> Tiempo de actualizacion. Buscando nueva frase...");
                fetchNewQuote();
                lastUpdateMillis = currentMillis;
            }
            
            if(aux >= refresh){
                copiar_a_bits(bits, offset);
                offset++;
                if(offset >= textoAncho) { offset = 0; }
                aux = 0;
            }
            aux++;
            break;
    }
}
