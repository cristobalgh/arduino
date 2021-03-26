#define termo 13 //A13 entrada termometro
float temperatura;
const byte numReadings = 10; // number of readings for smoothing (max 64)
int readings[numReadings]; // readings from the analogue input
byte indice = 0; // indice of the current reading
unsigned int total = 0; // running total

void setup(){
  // Debug console
  Serial.begin(115200);
  for (indice = 0; indice < numReadings; indice++) { // fill the array for faster startup
    readings[indice] = analogRead(termo);
    delay(10); //for analog input to stabilize
    total = total + readings[indice];
  }
  indice = 0; // reset
}

void loop(){
  total = total - readings[indice]; // subtract the last reading
  readings[indice] = analogRead(termo); // one unused reading to clear ghost charge
  delay(5); // que se estabilice
  readings[indice] = analogRead(termo); // read from the sensor
  total = total + readings[indice]; // add the reading to the total
  indice += 1; // advance to the next position in the array
  if (indice >= numReadings) // if we're at the end of the array
    indice = 0; // wrap around to the beginning
    
  float celsius = total/numReadings/4095.0*3290/10-273.15; // convert value to temp


  //int rawvoltage= analogRead(termo);
  //float millivolts= (rawvoltage/4095.0) * 3290; //resolucion y Vcc lm335
  
  //float kelvin= (millivolts/10);
  //Serial.print(kelvin);
  //Serial.println(" degrees Kelvin");

  //float celsius= kelvin - 273.15;
  Serial.print(celsius);
  Serial.println(" degrees Celsius");
  //Serial.print(rawvoltage);

  //float fahrenheit= ((celsius * 9)/5 +32);
  //Serial.print(fahrenheit);
  //Serial.println(" degrees Fahrenheit");

  delay(2000);
}
