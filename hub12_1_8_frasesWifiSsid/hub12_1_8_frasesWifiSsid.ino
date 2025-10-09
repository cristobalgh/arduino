// P4.75 HUB12 1/8 scan 16x64 de un solo color, matrix control.
// scroll text maximo MAX_COLS, caracteres de 8 bits de ancho
// hotspot web server si no conoce el wifi, 192.168.4.1 le das
// la clave de los wifis que ve

#include "soc/gpio_struct.h"
#include <WiFi.h> 
#include <HTTPClient.h>
#include <AsyncTCP.h>           // REQUIRED for ESPAsyncWebServer
#include <ESPAsyncWebServer.h>  // REQUIRED for WiFi Manager
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

// ===== WIFI MANAGER GLOBALS (from first code) =====
// Name for the hotspot you'll connect to for configuration
const char* apSsid = "Maipo-Matrix-Config"; 
// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);
// Variables to store the credentials from the web form temporarily
String ssid_temp;
String password_temp;
// A flag to indicate if we've received credentials and should try to connect
bool shouldConnect = false; 

// HTML and JavaScript for the web page (from first code)
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html><head>
  <title>ESP32 WiFi Manager</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f4f4f4; }
    .container { max-width: 500px; margin: auto; background-color: #fff; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }
    h2 { color: #333; }
    select, input[type=password], input[type=submit] { width: 100%; padding: 12px; margin: 8px 0; display: inline-block; border: 1px solid #ccc; border-radius: 4px; box-sizing: border-box; }
    input[type=submit] { background-color: #4CAF50; color: white; border: none; cursor: pointer; }
    input[type=submit]:hover { background-color: #45a049; }
    .loader { border: 4px solid #f3f3f3; border-top: 4px solid #3498db; border-radius: 50%; width: 30px; height: 30px; animation: spin 2s linear infinite; margin: 20px auto; }
    @keyframes spin { 0% { transform: rotate(0deg); } 100% { transform: rotate(360deg); } }
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
          document.getElementById('loader').style.display = 'none';
          document.getElementById('wifiForm').style.display = 'block';
        });
    }
  </script>
</head>
<body onload="fetchSSIDs()">
  <div class="container">
    <h2>Configure WiFi</h2>
    <div id="loader" class="loader"></div>
    <form id="wifiForm" action="/connect" method="GET" style="display:none;">
      <label for="ssid">Choose a Network (SSID):</label>
      <select id="ssid" name="ssid"></select>
      <br><br>
      <label for="pass">Password:</label>
      <input type="password" id="pass" name="pass">
      <br><br>
      <input type="submit" value="Connect">
    </form>
  </div>
</body></html>
)rawliteral";

// ===== CREDENCIALES Y CONSTANTES DE RED (Fallbacks/Initial values from second code) =====
// These will only be used if NO credentials are saved in NVS.
const char* ssid_config = "mosqueton";
const char* password_config = "esmerilemelo";

// Buffer y constantes para almacenar la frase descargada
#define MAX_QUOTE_LENGTH 440
char currentQuote[MAX_QUOTE_LENGTH] = "    Inicializando matriz    ";

// URL de una API de frases
const char* QUOTE_API_URL = "http://api.forismatic.com/api/1.0/?method=getQuote&lang=en&format=text"; 

// Control de tiempo para la actualización (10 minutos en milisegundos)
unsigned long lastUpdateMillis = 0;
const long updateInterval = 10UL * 60UL * 1000UL; 

// ===== VARIABLES GLOBALES DE SCROLL Y MATRIZ (from second code) =====
#define MAX_CHARS_SUPPORTED 400
#define MAX_COLS (MAX_CHARS_SUPPORTED * 8) 

uint8_t matrizGlobal[FILAS][MAX_COLS];  
int textoAncho = 0;  
int fila  = 1;
int refresh = 310;   
int aux   = 0;
int offset  = 0;
uint8_t bits[FILAS][COLS]; 

//matrices auxiliares para antes de pasar a la matriz led de 1/8
uint8_t uno[8][16], dos[8][16], tres[8][16], cuatro[8][16],
        cinco[8][16], seis[8][16], siete[8][16], ocho[8][16];

const char *TEXTO_A_MOSTRAR = currentQuote;

// =========================================================
//                  FUNCIONES DE GRÁFICOS Y HW
// =========================================================

void subCopiar() {
  // los datos de 'bits' (16x64) a los 8 sub-buffers (8x16).
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
    else      GPIO.out_w1tc = mask;
  } else {
    uint32_t mask = (1UL << (pin - 32));
    if (val) GPIO.out1_w1ts.val = mask;
    else      GPIO.out1_w1tc.val = mask;
  }
}

void escribe() {
  subCopiar();
  // El resto de esta función maneja el control del HUB12
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

            // Esta línea asume que textoAncho nunca es 0.
            bits[r][c] = matrizGlobal[r][srcCol];
        }
    }
}

// =========================================================
//      FUNCIÓN DE ESCRITURA DE TEXTO A MATRIZ UNIFICADA
// =========================================================

void write_text(const char *text, bool stretch) {
    memset(matrizGlobal, 0, sizeof(matrizGlobal));

    int charWidth = 8;
    int textHeight = stretch ? 16 : 8; 
    int vOffset = (FILAS - textHeight) / 2;

    int len = strlen(text);
    int maxChars = MAX_COLS / charWidth;
    if (len > maxChars) len = maxChars;

    textoAncho = len * charWidth;  // ancho real en píxeles

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
//                  FUNCIÓN DE WIFI MANAGER
// =========================================================

// Setup the ESP32 as an Access Point
void setupAP() {
    Serial.println("Starting AP for WiFi configuration...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(apSsid);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Serve the main HTML page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html);
    });

    // Handle the WiFi scan request
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

    // Handle the connection request
    server.on("/connect", HTTP_GET, [](AsyncWebServerRequest *request){
      if(request->hasParam("ssid") && request->hasParam("pass")) {
        // Get credentials from the request and store them in the global variables
        ssid_temp = request->getParam("ssid")->value();
        password_temp = request->getParam("pass")->value();
        
        String response = "<html><body><h2>Configuration Received</h2><p>Attempting to connect to your network. The hotspot will now shut down.</p></body></html>";
        request->send(200, "text/html", response);
        
        // Stop the access point and server
        WiFi.softAPdisconnect(true);
        server.end();

        shouldConnect = true; // Set flag to exit the AP loop and try connecting
        Serial.println("Shutting down AP and preparing to connect to WiFi...");
        
      } else {
        request->send(400, "text/plain", "Bad Request");
      }
    });

    // Start the server
    server.begin();
    Serial.println("AP and Web Server started.");

    // Loop until credentials are received
    while (!shouldConnect) {
      // Keep the matrix running while in AP mode
      // This is crucial to keep the display active
      copiar_a_bits(bits, offset); // Update display buffer
      escribe(); // Draw on matrix
      delay(2); // Short delay to prevent watchdog reset and improve stability
    }
}

// =========================================================
//                      FUNCIONES DE RED
// =========================================================

// connectToWiFi is modified to take optional credentials
void connectToWiFi(const char* connectSsid = nullptr, const char* connectPassword = nullptr) {
    // 1. Determine which credentials to use
    const char* targetSsid;
    const char* targetPassword;
    
    if (connectSsid != nullptr && connectPassword != nullptr) {
        // Use credentials from the web form
        targetSsid = connectSsid;
        targetPassword = connectPassword;
        WiFi.mode(WIFI_STA); // Switch to Station mode
    } else {
        // Try to reconnect using previously saved NVS credentials (or the hardcoded ones if NVS is empty)
        // If NVS has credentials, WiFi.begin() with no arguments will use them.
        WiFi.mode(WIFI_STA); 
        // We use the hardcoded ones as a fallback/initial try only if nothing is in NVS, 
        // but WiFi.begin() without args handles NVS, so we rely on that.
        // For the *very first* run, we'll try the hardcoded ones in setup() if auto-connect fails.
        targetSsid = ""; 
        targetPassword = "";
    }
    
    Serial.print("Conectando a ");
    
    if (targetSsid[0] != '\0') {
      Serial.println(targetSsid);
      WiFi.begin(targetSsid, targetPassword); 
    } else {
      // Attempt to connect with saved credentials
      Serial.println("saved/default network...");
      WiFi.begin(); 
    }
    
    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 40) { // Increased attempts for a better chance
        delay(500);
        Serial.print(".");
        
        // *** IMPORTANT: Keep matrix scrolling during connection attempts ***
        copiar_a_bits(bits, offset);
        escribe();
        // Scroll logic (copying from loop)
        aux++;
        if(aux >= refresh){
            offset++;
            if(offset >= textoAncho) {
                offset = 0;
            }
            aux = 0;
        }
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nWiFi Conectado!");
        Serial.print("Dirección IP: ");
        Serial.println(WiFi.localIP());
    } else {
        Serial.println("\nFALLÓ LA CONEXIÓN WiFi.");
    }
}


void fetchNewQuote() {
    if (WiFi.status() != WL_CONNECTED) {
        // Try to connect using saved/default credentials if currently disconnected
        connectToWiFi(); 
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("No hay conexión WiFi, saltando la actualización.");
            return;
        }
    }

    HTTPClient http;
    http.begin(QUOTE_API_URL);
    int httpCode = http.GET();

    if (httpCode > 0) {
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            
            // 1. Limpiar el payload y agregar espacios para el scroll
            String formattedQuote = "    " + payload + "    ";
            
            // 2. Copiar al buffer global, asegurando el límite.
            formattedQuote.toCharArray(currentQuote, MAX_QUOTE_LENGTH);
            
            Serial.print("Nueva Frase: ");
            Serial.println(currentQuote);

            // 3. Re-dibujar la matriz con la nueva frase
            write_text(TEXTO_A_MOSTRAR, false); 
            
            // 4. Reiniciar el scroll para la nueva frase
            offset = 0; 
            
        } else {
            Serial.printf("Error en la solicitud HTTP. Código: %d\n", httpCode);
        }
    } else {
        Serial.printf("Fallo en la conexión HTTP: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
}

// =========================================================
//                      SETUP Y LOOP
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

    // Poner pines en estado inicial bajo
    fastWrite(oe_pin, 1); //1 es apagado
    fastWrite(a_pin, 0);
    fastWrite(b_pin, 0);
    fastWrite(c_pin, 0);
    fastWrite(sclk_pin, 0);
    fastWrite(clk_pin, 0);
    fastWrite(dato_pinR, 0);
}

void setup() {
    inicializar();
    
    // --- Lógica del WiFi Manager Integrada ---
    
    // 1. Try to connect with saved credentials first
    WiFi.mode(WIFI_STA);
    WiFi.begin(); // Uses credentials saved in NVS (if any)
    
    int autoConnectAttempts = 0;
    while (WiFi.status() != WL_CONNECTED && autoConnectAttempts < 15) {
      delay(500);
      Serial.print("*");
      autoConnectAttempts++;
    }
    
    // 2. If auto-connect failed, start the AP for configuration
    if (WiFi.status() != WL_CONNECTED) {
        // Use the initial message about the AP for scrolling
        char apMsg[MAX_QUOTE_LENGTH];
        snprintf(apMsg, MAX_QUOTE_LENGTH, "19216841"); //alcanza a escribir esto en la matriz para una pista de dde configurar
        write_text(apMsg, false); 
        
        Serial.println("\nAuto-connect failed. Starting AP.");
        setupAP(); // This function blocks until credentials are received

        // After setupAP returns, attempt to connect with the new credentials
        if (shouldConnect) {
             // This call also saves the credentials to NVS implicitly
             connectToWiFi(ssid_temp.c_str(), password_temp.c_str()); 
        }
    }
    
    // 3. Update the initial scrolling text based on connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi, proceeding with quote fetch.");
    } else {
        // If still not connected (even after AP), revert to a static message
        char failMsg[MAX_QUOTE_LENGTH];
        snprintf(failMsg, MAX_QUOTE_LENGTH, "    ERROR: Sin conexión a Internet. Último SSID intentado: %s    ", ssid_temp.length() > 0 ? ssid_temp.c_str() : "Guardado/Por Defecto");
        write_text(failMsg, false); 
        TEXTO_A_MOSTRAR = currentQuote; // Ensure it points to the buffer
    }

    // 4. Get the first quote if connected. If not, the error message scrolls.
    fetchNewQuote();  

    // 5. Initial draw
    write_text(TEXTO_A_MOSTRAR, false); 
    copiar_a_bits(bits, offset);
}

void loop() {
    // === 1. Lógica de Actualización Periódica (no bloqueante) ===
    unsigned long currentMillis = millis();
    if (currentMillis - lastUpdateMillis >= updateInterval) {
        Serial.println(">>> Tiempo de actualización. Buscando nueva frase...");
        fetchNewQuote();
        lastUpdateMillis = currentMillis;
    }
    
    // === 2. Lógica de Scroll y Dibujo (controla la velocidad del scroll) ===
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
    
    // === 3. Dibujar en la matriz ===
    escribe();
}
