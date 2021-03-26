//con el tmp36
#include <BlynkSimpleEsp32.h>

const int analin = 36; //A36 entrada analoga
float celsius = 25.0; //para inicializar valores
const byte numReadings = 3; //number of readings for smoothing (max 64)
int readings[numReadings]; //readings from the analogue input
byte indice = 0; //indice de lectura actual
unsigned int total = 0; //total acumulado
const float factor = 0.39; //factor correccion analin
const int maxai = 1024; //max valor analin
const int resolu = 10; //9-12 9->511 12->4095
const float vmax = 3.3; //v+ esp32

char auth[] = "15dd30c052c540ed857fc7c37f3b8ee6";
char ssid[] = "mosqueton";
char pass[] = "Esmerilemelo1122";

BlynkTimer timer;
BlynkTimer timer2;
BlynkTimer timer3;
BlynkTimer timer4;

void setup(){
  analogReadResolution(resolu); //9-12 9->511 12->4095
  Serial.begin(115200);
  for (indice = 0; indice < numReadings; indice++) {
    // fill the array for faster startup
    delay(5); //for analog input to stabilize
    readings[indice] = analogRead(analin);
    total = total + readings[indice];
  }
  indice = 0; // reset
  Serial.println("hola...");

  Blynk.begin(auth, ssid, pass);
  // Setup a function to be called every x seconds
  timer.setInterval(300000L, enviadatos); //5min
  timer2.setInterval(120000L, revisaconexion); //120 segundos
  timer3.setInterval(3600000L, reinicia); //si nada funciona...(1h)
  timer4.setInterval(10000L, lee); //10 segundos lee analogo
}

void loop(){
  timer.run();  // envia datos
  timer2.run(); // revisa conexion
  timer3.run(); // reinicia con esp.restart()
  timer4.run(); // lee analogo, suma a total y pasa a °C

  if (Blynk.connected())
    Blynk.run();
}

void lee(){
  total = total - readings[indice]; // subtract the last reading
  readings[indice] = analogRead(analin); // one unused reading to clear ghost charge
  delay(50); // que se estabilice
  readings[indice] = analogRead(analin); // read from the sensor
  int i = 0;
  for(i=0; i<numReadings; i++)
    Serial.println(readings[i]);
  total = total + readings[indice]; // add the reading to the total
  indice += 1; // advance to the next position in the array
  if (indice >= numReadings) // if we're at the end of the array
    indice = 0; // wrap around to the beginning
  celsius = ((((total/numReadings) / (float)maxai) * vmax) - factor) * 100; // convert value to temp
  celsius = round(celsius*10)/10.0;
  Serial.print("Temperatura: ");
  Serial.print(celsius);
  Serial.println(" ºC");
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
