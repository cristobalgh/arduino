/*Para poder usar el pin5 como entrada analoga o binaria, 
 * el voltaje tiene que ser sobre 3 V aprox, sino se resetea
 * es el RST pin... 
 * Para arreglar esto y poder usar el pin5
 * ver link guardado:
 * http://thetoivonen.blogspot.com/2015/12/fixing-pin-p5-or-6-on-digispark-clones.html
 * =)
 */
#include "Stepper_28BYJ_48.h"
#include <PID_v1.h>

#define Spoint 25 //ºC temp agua
#define minLlave 25 //pasos pos minimo llave
#define maxLlave 190  //pasos pos max llave
#define sampleT 1000  //actualizacion PID en ms
#define kape 1 //Kp
#define kai 1 //Ki
#define kade 0 //Kd
int posact;

//Define Variables we'll be connecting to
double Setpoint, Input, Output, salida;

//Specify the links and initial tuning parameters
double Kp=kape, Ki=kai, Kd=kade;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);

#define boton 0 //pin5 del reset, hay que reparar para poder usar
                //switch posicion minima llave gas P4
#define p0 8
#define p1 9
#define p2 10
#define p3 11
#define termo A1 //A1 o 1 es p2 5v unico, 0 es p5 3.6V max
//#define steps 50
//#define delayy 500

Stepper_28BYJ_48 stepper(p3,p2,p1,p0); //in4, in3, in2, in1

void setup(){
  //pinMode(boton, INPUT); //para el attiny85
  Serial.begin(9600);
  pinMode(boton, INPUT_PULLUP);
  //digitalWrite(boton,HIGH);//este truco es para que se active el PULLUP attiny 85
  pinMode(p0, OUTPUT);
  pinMode(p1, OUTPUT);
  pinMode(p2, OUTPUT);
  pinMode(p3, OUTPUT);
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH); //alim termometro

  myPID.SetSampleTime(sampleT);
  myPID.SetOutputLimits(minLlave, maxLlave);
  Setpoint = Spoint;
  //turn the PID on
  myPID.SetMode(AUTOMATIC);
  Serial.print("Buscando 0");
  while(digitalRead(boton) == HIGH ){ //cuando boton se va a cero motor en minLlave
    stepper.step(-1);
  }
  posact = minLlave;
}

void loop()
{
  Input = lee(termo); //convierte a ºC
  myPID.Compute();
  imprime();

  if(posact != Output) //me tengo que mover
  {
    stepper.step(Output-posact);
    posact = Output;
  }
  delay(2000);
}

float lee(float entrada)
{
  delay(5);
  float lectura = analogRead(entrada);
  delay(5);
  float voltaje = lectura * 5.0;
  voltaje /= 1024.0;
  float tempC = (voltaje -0.5) * 100;
  return tempC;
}

void imprime()
{
  Serial.print("sp:");
  Serial.print(" ");
  Serial.print(Setpoint);
  Serial.print(" ");
  Serial.print("in:");
  Serial.print(" ");
  Serial.print(lee(termo));
  Serial.print(" ");
  Serial.print("out:");
  Serial.print(" ");
  Serial.print(Output);
  Serial.print(" ");
  Serial.print("pac:");
  Serial.print(" ");
  Serial.println(posact);
  delay(1);
}
