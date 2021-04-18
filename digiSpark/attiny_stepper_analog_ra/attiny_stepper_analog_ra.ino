/*Para poder usar el pin5 como entrada analoga o binaria, 
 * el voltaje tiene que ser sobre 3 V aprox, sino se resetea
 * es el RST pin... 
 * Para arreglar esto y poder usar el pin5
 * ver link guardado:
 * http://thetoivonen.blogspot.com/2015/12/fixing-pin-p5-or-6-on-digispark-clones.html
 * =)
 */
#include "Stepper_28BYJ_48.h"

#define boton 5 //pin5 del reset, hay que reparar para poder usar
#define p0 0
#define p1 1
#define p2 3
#define p3 4
#define termo A1 //A1 o 1 es p2 5v unico, 0 es p5 3.6V max
#define steps 50
#define delayy 500

Stepper_28BYJ_48 stepper(p3,p2,p1,p0); //in4, in3, in2, in1

void setup(){
  pinMode(boton, INPUT);
  digitalWrite(boton,HIGH);//este truco es para que se active el PULLUP
  pinMode(p0, OUTPUT);
  pinMode(p1, OUTPUT);
  pinMode(p2, OUTPUT);
  pinMode(p3, OUTPUT);
}

void loop(){
  int sensorValue = 0; //entrada analoga
  int switchValue = 0; //boton
  sensorValue = analogRead(termo);
  stepper.step(sensorValue);
  delay(delayy);
  
  switchValue=digitalRead(boton);
  if(switchValue){
    stepper.step(steps);
    delay(delayy);
  }
  else {
    stepper.step(-steps);
    delay(delayy);
  }
}
