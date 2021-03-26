//initializes/defines the output pin of the LM335 temperature sensor
int outputPin= 0;
//this sets the ground pin to LOW and the input voltage pin to high
void setup()
{
Serial.begin(9600);
}

//main loop
void loop()
{
int rawvoltage= analogRead(outputPin);
float millivolts= (rawvoltage/1024.0) * 5000;
float kelvin= (millivolts/10);
Serial.print(kelvin);
Serial.println(" degrees Kelvin");

float celsius= kelvin - 273.15;
Serial.print(celsius);
Serial.println(" degrees Celsius");

float fahrenheit= ((celsius * 9)/5 +32);
Serial.print(fahrenheit);
Serial.println(" degrees Fahrenheit");

delay(3000);
}
