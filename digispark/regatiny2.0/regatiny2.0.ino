//esta version cierra el rele con un HIGH no como el rele
//que viene con placa y que cierra con un LOW
#include "tinysnore.h"

int cuentatotal = 0;
//int periodo     = 1;//120; //m, cada cuanto quiero que de el agua, 2 horas
int tsnore      = 500; //ms, pequeño para que funcione la interrupcion
int cuenta      = 1440; //periodo(m) * 60 * 1000 / tsnore;
//a contar para cumplir el largo de ciclo deseado
int triego      = 2;//15; //s, tiempo que se da el agua cada vez

int rele        = 0; //salida a la que esta conectada el rele
int boton       = 2; //pin 2 despierta por interrupt 0
int estadoboton = HIGH; //pullup a 5V
int led         = 1; //debug

void setup() {
  pinMode(led, OUTPUT); //debug
  pinMode(rele, OUTPUT);
  pinMode(boton, INPUT_PULLUP); //cuando apreto boton se va a 0V
  cortaelagua();
}

void loop() {
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(0, despierta, FALLING);
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  snore(tsnore); //va contando
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(0);

  if (estadoboton == LOW){
    daelagua();
    snore(triego*1000);
    estadoboton = HIGH;
  }
  else{
    cortaelagua();
  }

  cuentatotal ++;

  if (cuentatotal == cuenta) {
    daelagua();
    snore(triego*1000);       //riego
    cortaelagua();
    cuentatotal = 0;
  }
}

void daelagua(){
  digitalWrite(led, HIGH);
  digitalWrite(rele, HIGH);
}

void cortaelagua(){
  digitalWrite(led, LOW);
  digitalWrite(rele, LOW);
}

void despierta(){
  estadoboton = LOW;
}
