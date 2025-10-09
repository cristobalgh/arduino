// =================================================================
// CÓDIGO FINAL LIMPIO: MATRIZ LED HUB12 CON DUAL-MODE WIFI Y OVERRIDE WEB
// Se han eliminado todos los caracteres extendidos (non-ASCII)
// =================================================================

#include "soc/gpio_struct.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "letras.h"

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

// ===== ESTADOS Y CONSTANTES GLOBALES =====
const char* apSsid = "Maipo-Matrix-Config";
const char* apPassword = "12345678"; // Contraseña para el Hotspot
AsyncWebServer server(80);

// Buffer para la frase actual
#define MAX_QUOTE_LENGTH 440
char currentQuote[MAX_QUOTE_LENGTH] = "    Inicializando matriz    ";
const char* TEXTO_A_MOSTRAR = currentQuote;

// URL de una API de frases
const char* QUOTE_API_URL = "http://api.forismatic.com/api/1.0/?method=getQuote&lang=en&format=text";

// Control de tiempo para la actualización de la API (10 minutos)
unsigned long lastApiUpdateMillis = 0;
const long updateApiInterval = 10UL * 60UL * 1000UL;

// Control de tiempo para la frase personalizada (30 segundos)
unsigned long customPhraseStartMillis = 0;
const long customPhraseDuration = 30UL * 1000UL;
bool isCustomPhraseActive = false;

// ===== VARIABLES GLOBALES DE SCROLL Y MATRIZ =====
#define MAX_CHARS_SUPPORTED 400
#define MAX_COLS (MAX_CHARS_SUPPORTED * 8)

uint8_t matrizGlobal[FILAS][MAX_COLS];
int textoAncho = 0;
int fila  = 1;
int refresh = 310;
int aux   = 0;
int offset  = 0;
uint8_t bits[FILAS][COLS];

uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16];

// =========================================================
//                   HTML CON SOBREESCRITURA DE FRASE
// =========================================================

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>Maipo Matrix Manager</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f4f4f4; }
    .container { max-width: 500px; margin: 15px auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h2 { color: #333; border-bottom: 2px solid #ccc; padding-bottom: 10px; }
    select, input[type=password], input[type=submit], textarea { width: 100%; padding: 12px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
    input[type=submit] { background-color: #2196F3; color: white; border: none; cursor: pointer; }
    input[type=submit]:hover { background-color: #0b7dda; }
  </style>
  <script>
    function fetchSSIDs() {
      fetch('/scan')
        .then(response => response.json())
        .then(data => {
          let select = document.getElementById('ssid');
          select.innerHTML = '';
          data.ssids.forEach(ssid => {
            let option = document.createElement('option');
            option.value = ssid;
            option.textContent = ssid;
            select.appendChild(option);
          });
        });
    }
  </script>
</head>
<body onload="fetchSSIDs()">
  <div class="container">
    <h2>⚙️ Configurar WiFi</h2>
    <form action="/connect" method="GET">
      <label for="ssid">Red WiFi (SSID):</label>
      <select id="ssid" name="ssid"></select>
      <label for="pass">Contraseña:</label>
      <input type="password" id="pass" name="pass">
      <input type="submit" value="Conectar y Guardar">
    </form>
  </div>

  <div class="container">
    <h2>✏️ Frase Personalizada (30s)</h2>
    <form action="/sendtext" method="POST">
      <label for="custom_text">Introduce tu frase:</label>
      <textarea id="custom_text" name="custom_text" rows="3" maxlength="400" required></textarea>
      <input type="submit" value="Mostrar por 30 segundos">
    </form>
  </div>
</body></html>
)rawliteral";

// =========================================================
//                   FUNCIONES DE GRÁFICOS Y HW
// =========================================================

void copySubMatrix(uint8_t dst[8][16], uint8_t src[FILAS][COLS], int startRow, int startCol) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 16; c++) {
            dst[r][c] = src[startRow + r][startCol + c];
        }
    }
}

void subCopiar() {
  copySubMatrix(uno,   bits, 8,  0);
  copySubMatrix(dos,   bits, 0,  0);
  copySubMatrix(tres,  bits, 8, 16);
  copySubMatrix(cuatro, bits, 0, 16);
  copySubMatrix(cinco,  bits, 8, 32);
  copySubMatrix(seis,   bits, 0, 32);
  copySubMatrix(siete,  bits, 8, 48);
  copySubMatrix(ocho,   bits, 0, 48);
}

void eligeFila(bool a,bool b,bool c) {
  fastWrite(a_pin,a);
  fastWrite(b_pin,b);
  fastWrite(c_pin,c);
}

// ===== Fast GPIO (usando registros del ESP32) =====
inline void fastWrite(uint8_t pin, bool val) {
  if (pin < 32) {
    uint32_t mask = (1UL << pin);
    if (val) GPIO.out_w1ts = mask;
    else      GPIO.out_w1tc = mask;
  } else {
    uint32_t mask = (1UL << (pin - 32));
    if (val) GPIO.out1_w1ts.val = mask;
    else      GPIO.out1_w1tc.val = mask;
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

            bits[r][c] = matrizGlobal[r][srcCol];
        }
    }
}

void write_text(const char *text, bool stretch) {
    memset(matrizGlobal, 0, sizeof(matrizGlobal));

    int charWidth = 8;
    int textHeight = stretch ? 16 : 8;
    int vOffset = (FILAS - textHeight) / 2;

    int len = strlen(text);
    int maxChars = MAX_COLS / charWidth;
    if (len > maxChars) len = maxChars;

    // Calcular el ancho del texto y asegurar un mínimo para scroll continuo
    textoAncho = len * charWidth;
    if (textoAncho < COLS * 2) {
        // Asegura que haya suficiente espacio en blanco para que el texto se "despegue" de sí mismo
        textoAncho = (len + (COLS/charWidth)) * charWidth;
    }


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
//                   FUNCIONES DE RED Y API
// =========================================================

void connectToWiFi(const char* connectSsid = nullptr, const char* connectPassword = nullptr) {
    if (connectSsid != nullptr && connectSsid[0] != '\0') {
        WiFi.begin(connectSsid, connectPassword);
        Serial.print("Intentando conectar a: ");
        Serial.println(connectSsid);
    } else {
        if (WiFi.status() != WL_CONNECTED) {
            WiFi.begin(); // Intenta con credenciales guardadas en NVS
            Serial.println("Intentando auto-conexión con credenciales guardadas.");
        }
    }
}

void fetchNewQuote() {
    if (WiFi.status() != WL_CONNECTED) {
        connectToWiFi(); // Intenta reconectar
        Serial.println("No hay conexión WiFi para la API.");
        return;
    }

    HTTPClient http;
    http.begin(QUOTE_API_URL);
    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        
        // Formato y copia con espacios de scroll
        String formattedQuote = "    " + payload + "    ";
        formattedQuote.toCharArray(currentQuote, MAX_QUOTE_LENGTH);
        
        Serial.print("Nueva Frase API: ");
        Serial.println(currentQuote);

        write_text(TEXTO_A_MOSTRAR, false);
        offset = 0; // Reiniciar scroll
    } else {
        Serial.printf("Error HTTP al obtener frase: %d\n", httpCode);
    }

    http.end();
}

// =========================================================
//                   FUNCIONES DEL SERVIDOR WEB
// =========================================================

void setupWebServer() {
    // Modo Dual: AP para la configuración web y STA para internet
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(apSsid, apPassword);
    Serial.print("Hotspot AP iniciado. IP: ");
    Serial.println(WiFi.softAPIP());

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html);
    });

    server.on("/scan", HTTP_GET, [](AsyncWebServerRequest *request){
      String json = "{\"ssids\":[";
      int n = WiFi.scanNetworks();
      for (int i = 0; i < n; ++i) {
        json += "\"" + WiFi.SSID(i) + "\"";
        if (i < n - 1) {
          json += ",";
        }
      }
      json += "]}";
      request->send(200, "application/json", json);
    });

    server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request){
      if(request->hasParam("ssid") && request->hasParam("pass")) {
        String ssid = request->getParam("ssid")->value();
        String password = request->getParam("pass")->value();
        
        connectToWiFi(ssid.c_str(), password.c_str()); 
        
        String response = "<html><body><h2>Conexión Iniciada</h2><p>Intentando conectar a su red. Regrese a 192.168.4.1 en unos segundos para verificar.</p></body></html>";
        request->send(200, "text/html", response);
      } else {
        request->send(400, "text/plain", "Bad Request");
      }
    });

    server.on("/sendtext", HTTP_POST, [](AsyncWebServerRequest *request){
        if(request->hasParam("custom_text", true)) {
            String customText = request->getParam("custom_text", true)->value();
            
            // 1. Formatear y copiar al buffer global
            String formattedText = "    (USUARIO) " + customText + "    ";
            formattedText.toCharArray(currentQuote, MAX_QUOTE_LENGTH);
            
            // 2. Activar el modo de frase personalizada
            isCustomPhraseActive = true;
            customPhraseStartMillis = millis();
            
            // 3. Prepara el texto para el scroll
            write_text(TEXTO_A_MOSTRAR, false);
            offset = 0;
            
            Serial.print("Frase personalizada activada: ");
            Serial.println(customText);

            String response = "<html><body><h2>Frase Recibida</h2><p>Su frase se mostrará por 30 segundos. El sistema volverá a las frases de internet automáticamente después de ese tiempo.</p></body></html>";
            request->send(200, "text/html", response);
        } else {
            request->send(400, "text/plain", "Falta el parámetro 'custom_text'.");
        }
    });

    server.begin();
    Serial.println("Web Server iniciado.");
}

// =========================================================
//                       SETUP Y LOOP
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

    // Poner pines en estado inicial
    fastWrite(oe_pin, 1); // 1 es apagado
    fastWrite(a_pin, 0);
    fastWrite(b_pin, 0);
    fastWrite(c_pin, 0);
    fastWrite(sclk_pin, 0);
    fastWrite(clk_pin, 0);
    fastWrite(dato_pinR, 0);
}

void setup() {
    inicializar();
    
    // 1. Configurar y encender el servidor web en modo Dual (AP + STA)
    setupWebServer();

    // 2. Inicializar la interfaz STA para que intente reconectar a la red guardada
    connectToWiFi(); 
    
    // 3. Obtener la primera frase de la API inmediatamente si es posible
    fetchNewQuote();
    lastApiUpdateMillis = millis();
    
    // 4. Mostrar el mensaje inicial
    write_text(TEXTO_A_MOSTRAR, false);
    copiar_a_bits(bits, offset);
}

void loop() {
    unsigned long currentMillis = millis();
    
    // === 1. Lógica de Override de Frase Personalizada (30s) ===
    if (isCustomPhraseActive) {
        if (currentMillis - customPhraseStartMillis >= customPhraseDuration) {
            isCustomPhraseActive = false;
            Serial.println(">>> Tiempo de frase personalizada terminado. Volviendo a la API.");
            // Forzar una actualización de la API para que el cambio sea inmediato
            fetchNewQuote(); 
            lastApiUpdateMillis = currentMillis;
        }
    }
    
    // === 2. Lógica de Actualización Periódica de la API (solo si NO hay override) ===
    if (!isCustomPhraseActive && (currentMillis - lastApiUpdateMillis >= updateApiInterval)) {
        Serial.println(">>> Tiempo de actualización. Buscando nueva frase de API...");
        fetchNewQuote();
        lastApiUpdateMillis = currentMillis;
    }
    
    // === 3. Lógica de Scroll y Dibujo ===
    if(aux >= refresh){
        copiar_a_bits(bits, offset);
        
        offset++;
        
        // Reinicio de scroll
        if(offset >= textoAncho) {
            offset = 0;
        }
        
        aux = 0;
    }
    aux++;
    
    // Dibujar en la matriz
    escribe();
}
