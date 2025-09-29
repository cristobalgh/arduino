#include "LowPower.h"

int cuentatotal = 0;
int periodo = 120;               //m, cada cuanto quiero que de el agua, 120s 
int cuenta = periodo * 60 / 8;  //s, valor a contar para cumplir el largo de ciclo deseado
int triego = 15;                //s, tiempo que se da el agua cada vez

int vcc = 8;
int gnd = 9;
int rele = 10;  //salida a la que esta conectada el rele
int boton = 2;  //pin 2 despierta por interrupt 0
int estadoboton = HIGH;
int led = 13;   //debug

void despierta()
{
  estadoboton = LOW;
  // Just a handler for the pin interrupt.
}

void setup() {
  pinMode(led, OUTPUT);   //debug
  pinMode(rele, OUTPUT);
  pinMode(boton, INPUT);
  pinMode(vcc, OUTPUT);
  pinMode(gnd, OUTPUT);
  digitalWrite(led, HIGH);  //debug
  digitalWrite(rele, HIGH); //parte apagado (se prende con 0)
  digitalWrite(vcc, HIGH);
  digitalWrite(gnd, LOW);
}

void loop() {
  // Allow wake up pin to trigger interrupt on low.
  attachInterrupt(0, despierta, LOW);//interrupt del boton cuando se va a GND
  // Enter power down state with ADC and BOD module disabled.
  // Wake up when wake up pin is low.
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  // Disable external pin interrupt on wake up pin.
  detachInterrupt(0);

  if (estadoboton == LOW) {
    digitalWrite(led, LOW);   //si esta apretado el boton, da el agua
    digitalWrite(rele, LOW);  //debug
    estadoboton = HIGH;
  }
  else {
    digitalWrite(led, HIGH);
    digitalWrite(rele, HIGH);   //debug
  }

  cuentatotal ++;

  if (cuentatotal <= cuenta) {
    digitalWrite(rele, LOW);    //doy el agua
    digitalWrite(led, LOW);     //debug
    delay(triego * 1000);       //riego
    digitalWrite(rele, HIGH);   //corto el agua
    digitalWrite(led, HIGH);    //debug
    cuentatotal = 0;
  }
}
