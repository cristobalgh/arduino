#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#define BLYNK_PRINT Serial

const int analin = 36; //A36 entrada analoga
float celsius = 25.0;
const byte numReadings = 32; // number of readings for smoothing (max 64)
int readings[numReadings]; // readings from the analogue input
byte indice = 0; // indice of the current reading
unsigned int total = 0; // running total

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "15dd30c052c540ed857fc7c37f3b8ee6";
char ssid[] = "mosqueton";
char pass[] = "esmerilemelo";

BlynkTimer timer;
BlynkTimer timer2;
BlynkTimer timer3;

void setup(){
  analogReadResolution(9); //9-12 9->511 12->4095
  Serial.begin(115200);
  for (indice = 0; indice < numReadings; indice++) {
    // fill the array for faster startup
    delay(5); //for analog input to stabilize
    readings[indice] = analogRead(analin);
    total = total + readings[indice];
  }
  indice = 0; // reset

  Blynk.begin(auth, ssid, pass);

  // Setup a function to be called every x seconds
  timer.setInterval(10000L, enviadatos); //10 segundos
  timer.setInterval(60000L, revisaconexion); //60 segundos
  timer.setInterval(3600000L, reinicia); //si nada funciona...(1h)
}

void loop(){
  total = total - readings[indice]; // subtract the last reading
  readings[indice] = analogRead(analin); // one unused reading to clear ghost charge
  delay(5); // que se estabilice
  readings[indice] = analogRead(analin); // read from the sensor
  total = total + readings[indice]; // add the reading to the total
  indice += 1; // advance to the next position in the array
  if (indice >= numReadings) // if we're at the end of the array
    indice = 0; // wrap around to the beginning
  celsius = total/numReadings/511.0*3290/10-273.1; // convert value to temp
  celsius = round(celsius*10)/10.0;

  if (Blynk.connected()){
    Blynk.run();
  }
  timer.run(); // Initiates BlynkTimer
  timer2.run();
  timer3.run(); // reinicia con esp.restart()
}

void reinicia(){
  Serial.println("Reinicio");
  if(!Blynk.connected()){ //ultima oportunidad
    Serial.println("Reinicio con ESP.restart()");
    delay(5);
    ESP.restart();
  }
}

void revisaconexion(){
  if(!Blynk.connected()){
    Serial.println("No conectado al servidor Blynk");
    Blynk.disconnect();
    Blynk.connect();
  }
  else{
    Serial.println("Conectado al servidor Blynk");
  }
}

void enviadatos(){
//You can send any value at any time.
//Please don't send more that 10 values per second.
//float valor = random(10.0);
//Blynk.virtualWrite(V5, valor);
//Serial.print("Valor: ");
//Serial.println(valor);
  Blynk.virtualWrite(V5, celsius);
  Serial.print("Temperatura: ");
  Serial.println(celsius,1);
}
