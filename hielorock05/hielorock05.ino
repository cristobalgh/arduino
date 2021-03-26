#include <BlynkSimpleEsp32.h>

const int analin = 36; //A36 entrada analoga
float celsius = 25.0;
const byte numReadings = 10; // number of readings for smoothing (max 64)
int readings[numReadings]; // readings from the analogue input
byte indice = 0; // index of the current reading
unsigned int total = 0; // running total

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
  timer.setInterval(60000L, enviadatos); //60 segundos
  timer2.setInterval(120000L, revisaconexion); //120 segundos
  timer3.setInterval(3600000L, reinicia); //si nada funciona...(1h)
}

void loop(){
  total = total - readings[indice]; // subtract the last reading
  readings[indice] = analogRead(analin); // one unused reading to clear ghost charge
  delay(500); // que se estabilice
  readings[indice] = analogRead(analin); // read from the sensor
  total = total + readings[indice]; // add the reading to the total
  indice += 1; // advance to the next position in the array
  if (indice >= numReadings) // if we're at the end of the array
    indice = 0; // wrap around to the beginning
  
  celsius = total/numReadings/511.0*3290/10-273.1; // convert value to temp
  celsius = round(celsius*10)/10.0;
  
  timer.run(); // Initiates BlynkTimer
  timer2.run();
  timer3.run(); // reinicia con esp.restart()

  if (Blynk.connected()){
    Blynk.run();
  }
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
  Blynk.virtualWrite(V5, celsius);
  Serial.print("Temperatura: ");
  Serial.println(celsius,1);
}
