/* Version 30.8.18
 *  Redboard
 *  LM335
 *  Motor stepper chico
 *  Suavizado de lectura analoga y promediado (numReadings)
 *  Se ven correctos los valores a numReading*delayloop/1000 segundos
 *  Imprime valores por puerto serial para debugging
 *  Importante fijar maxLlave antes para no forzar motor ni valvula calefon
 *  4.9: ajuste kp ki de 4 6 a 2 3 por sobre oscilaciones
 *  cambio sp a 35
 *  5.9 cambio sp a 37
 *  8.9 ki de 3 a 1 para ver si oscila menos
 *  9.9 vuelta a ki 4 y max 150 y sp 35
*/

#include "Stepper_28BYJ_48.h"
#include <PID_v1.h>

#define Spoint 35 //ºC temp agua salida
#define minLlave 0 //pasos pos minimo llave
#define maxLlave 150 //pasos pos max llave
#define sampleT 500 //actualizacion PID en ms
#define delayloop 200 //ms actualización loop, baja posible
#define kape 2 //Kp
#define kai 4 //Ki
#define kade 0 //Kd
#define boton 13 //boton de posición minima valvula gas
#define p0 8
#define p1 9
#define p2 10
#define p3 11
#define termo 0 //A0 entrada termometro
const byte numReadings = 5; // number of readings for smoothing (max 64)

int readings[numReadings]; // readings from the analogue input
byte index = 0; // index of the current reading
unsigned int total = 0; // running total
int posact=0; //posicion actual del motor
double Setpoint, Input, Output, salida; //Define Variables we'll be connecting to
double Kp=kape, Ki=kai, Kd=kade; //Specify the links and initial tuning parameters

PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
Stepper_28BYJ_48 stepper(p3,p2,p1,p0); //in4, in3, in2, in1

void setup(){
  Serial.begin(9600);
  for (index = 0; index < numReadings; index++) { // fill the array for faster startup
    readings[index] = analogRead(termo);
    delay(10); //for analog input to stabilize
    total = total + readings[index];
  }
  index = 0; // reset
  pinMode(boton, INPUT_PULLUP);
  pinMode(p0, OUTPUT);
  pinMode(p1, OUTPUT);
  pinMode(p2, OUTPUT);
  pinMode(p3, OUTPUT);
  pinMode(13, OUTPUT);
  myPID.SetSampleTime(sampleT);
  myPID.SetOutputLimits(minLlave, maxLlave);
  Setpoint = Spoint;
  myPID.SetMode(AUTOMATIC); //turn the PID on
  Serial.print("Buscando 0");
  while(digitalRead(boton) == HIGH ){ //cuando boton se va a cero motor en minLlave
    stepper.step(-1);
  }
  stepper.step(15);//para que no quede forzado
  posact = minLlave;
  Output = posact;
}

void loop()
{
  total = total - readings[index]; // subtract the last reading
  readings[index] = analogRead(termo); // one unused reading to clear ghost charge
  delay(5); // que se estabilice
  readings[index] = analogRead(termo); // read from the sensor
  total = total + readings[index]; // add the reading to the total
  index += 1; // advance to the next position in the array
  if (index >= numReadings) // if we're at the end of the array
    index = 0; // wrap around to the beginning
    
  //Serial.println(lee(termo));//lee, convierte a ºC e imprime.
  Input = total/numReadings/1024.0*5000/10-273.15; // convert value to temp
  myPID.Compute();
  imprime();

  if(int(posact) != int(Output)) //si me tengo que mover
  {
    Serial.println(Output-posact);
    stepper.step(int(Output)-int(posact));
    posact = Output;
    Serial.println(posact);
  }
  delay(delayloop);
}

float lee(int analoga)
{
  int rawvoltage= analogRead(analoga);
  float millivolts= (rawvoltage/1024.0) * 5000;
  float celsius= (millivolts/10) - 273.15;
  return celsius;
}

void imprime()
{
  Serial.print("sp:");
  Serial.print(" ");
  Serial.print(int(Setpoint));
  Serial.print(" ");
  Serial.print("in:");
  Serial.print(" ");
  Serial.print(int(Input));
  Serial.print(" ");
  Serial.print("out:");
  Serial.print(" ");
  Serial.print(int(Output));
  Serial.println(" ");
  delay(1);
}
