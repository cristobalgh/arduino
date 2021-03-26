#include <SPI.h>
#include <Ethernet.h>

byte mac[] = { 0x90, 0xA2, 0xDA, 0x0F, 0x16, 0xB2 };
int pinsonar = A0;
int llenado = 0;
// ThingSpeak Settings
char thingSpeakAddress[] = "api.thingspeak.com";
String writeAPIKey = "JSO7W87E6A4NJECA";
const int updateThingSpeakInterval = 60 * 1000;
// (number of seconds * 1000 = interval)
// Time interval in milliseconds to update ThingSpeak

// Variable Setup
long lastConnectionTime = 0;
boolean lastConnected = false;
int failedCounter = 0;
// Initialize Arduino Ethernet Client
EthernetClient client;

void setup(){
  // Start Serial for debugging on the Serial Monitor
  Serial.begin(9600);
  // Start Ethernet on Arduino
  startEthernet();
}
void loop(){
  // Read value from Analog Input Pin 0
  llenado = map(analogRead(pinsonar), 12, 507, 100, 0);
  String analogValue0 = String(llenado, DEC);
  
  // Print Update Response to Serial Monitor
  if (client.available()) {
    char c = client.read();
    Serial.print(c);
  }
  // Disconnect from ThingSpeak
  if (!client.connected() && lastConnected) {
    Serial.println("...disconnected");
    Serial.println();
    client.stop();
  }
  // Update ThingSpeak
  if (!client.connected() && (millis() - lastConnectionTime > updateThingSpeakInterval))
  {
    updateThingSpeak("field1=" + analogValue0);
  }
  // Check if Arduino Ethernet needs to be restarted
  if (failedCounter > 3){
    startEthernet();
  }
  lastConnected = client.connected();
}
void updateThingSpeak(String tsData) {
  if (client.connect(thingSpeakAddress, 80)) {
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    lastConnectionTime = millis();
    if (client.connected()) {
      Serial.println("Connecting to ThingSpeak...");
      Serial.println();
      failedCounter = 0;
    }
    else {
      failedCounter++;
      Serial.println("Connection to ThingSpeak failed (" + String(failedCounter, DEC) + ")");
      Serial.println();
    }
  }
  else {
    failedCounter++;
    Serial.println("Connection to ThingSpeak Failed (" + String(failedCounter, DEC) + ")");
    Serial.println();
    lastConnectionTime = millis();
  }
}
void startEthernet() {
  client.stop();
  Serial.println("Connecting Arduino to network...");
  Serial.println();
  delay(1000);
  // Connect to network amd obtain an IP address using DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP Failed, reset Arduino to try again");
    Serial.println();
  }
  else {
    Serial.println("Arduino connected to network using DHCP");
    Serial.println();
  }
  delay(1000);
}
