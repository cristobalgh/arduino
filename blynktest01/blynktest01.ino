#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "15dd30c052c540ed857fc7c37f3b8ee6";

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "mosqueton";
char pass[] = "esmerilemelo";

BlynkTimer timer;
BlynkTimer timer2;
BlynkTimer timer3;

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

void myTimerEvent(){
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  float valor = random(10.0);
  Blynk.virtualWrite(V5, valor);
  Serial.print("Valor: ");
  Serial.println(valor);
}

void setup(){
  // Debug console
  Serial.begin(115200);
  randomSeed(analogRead(0));

  Blynk.begin(auth, ssid, pass);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer.setInterval(10000L, myTimerEvent); //10 segundos
  timer.setInterval(60000L, revisaconexion); //60 segundos
  timer.setInterval(3600000L, reinicia); //si nada funciona...(1h)
}

void loop(){
  if (Blynk.connected()){
    Blynk.run();
  }
  timer.run(); // Initiates BlynkTimer
  timer2.run();
  timer3.run(); // reinicia con esp.restart()
}
