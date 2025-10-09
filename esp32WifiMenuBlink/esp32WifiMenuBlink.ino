#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Define the GPIO pin for the LED. Most dev boards have a built-in LED on pin 2.
const int ledPin = 2;

// Name for the hotspot you'll connect to for configuration
const char* apSsid = "ESP32-WiFi-Manager";

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);

// Variables to store the credentials in RAM (Note: Persistence is handled automatically by WiFi.begin())
String ssid;
String password;

// --- GLOBAL VARIABLES FOR PULSING BLINK PATTERN ---
// Define the min/max delay for the frequency range
const int MIN_DELAY_MS = 50;   // 10 Hz blink frequency (50ms on, 50ms off)
const int MAX_DELAY_MS = 500;  // 1 Hz blink frequency (500ms on, 500ms off)
int currentDelay = MAX_DELAY_MS; // Start slow (1 Hz)
int delayStep = -5;              // How much to change the delay by each loop. Negative to decrease delay (speed up)
// ---------------------------------------------------

// HTML and JavaScript for the web page
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


// Setup the ESP32 as an Access Point
void setupAP() {
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
      ssid = request->getParam("ssid")->value();
      password = request->getParam("pass")->value();
      
      String response = "<html><body><h2>Configuration Received</h2><p>Attempting to connect to your network. The hotspot will now shut down.</p></body></html>";
      request->send(200, "text/html", response);
      
      // Stop the access point and server
      WiFi.softAPdisconnect(true);
      server.end();

      // Switch to Station mode and attempt to connect (this call implicitly saves credentials to NVS if successful)
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid.c_str(), password.c_str());
      Serial.println("Shutting down AP and attempting to connect to WiFi...");
      
    } else {
      request->send(400, "text/plain", "Bad Request");
    }
  });

  // Start the server
  server.begin();
  Serial.println("AP and Web Server started.");
}

// -------------------------------------------------------------------------
void setup() {
  Serial.begin(115200); // Standard Baud rate for ESP32
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW); // Start with LED off

  // On every boot, immediately start in Access Point mode for configuration
  setupAP();
}

// -------------------------------------------------------------------------
void loop() {
  // Only execute the blinking pattern if connected to WiFi
  if(WiFi.status() == WL_CONNECTED){
    
    // 1. Blink the LED using the current delay
    digitalWrite(ledPin, HIGH);
    delay(currentDelay);
    digitalWrite(ledPin, LOW);
    delay(currentDelay);
    
    // 2. Update the delay for the next cycle
    currentDelay += delayStep;

    // 3. Check if we've hit the min/max limits and reverse the direction (delayStep)
    if (currentDelay <= MIN_DELAY_MS) {
      // Hit the fastest speed (10 Hz) -> start slowing down
      delayStep = abs(delayStep); // make it positive
      currentDelay = MIN_DELAY_MS; // Ensure it doesn't go below the limit
    } else if (currentDelay >= MAX_DELAY_MS) {
      // Hit the slowest speed (1 Hz) -> start speeding up
      delayStep = -abs(delayStep); // make it negative
      currentDelay = MAX_DELAY_MS; // Ensure it doesn't go above the limit
    }
  } 
  // If not connected, the LED remains off.
}
