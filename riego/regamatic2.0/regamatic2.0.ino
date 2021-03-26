#include "LowPower.h"
//para redboard y arduino uno y mini
//seleccionar programador ArduinoISP
//par el del depto, es placa arduino pro mini

int cuentatotal = 0;
int periodo     = 360; //m, cada cuanto quiero que de el agua.
//aumentado 11.11.19 de 120 a 240m
//aumentado 17.11.19 de 240 a 360m
int cuenta      = periodo * 60 / 8; //s, valor a contar para cumplir el largo de ciclo deseado
int triego      = 8; //s, tiempo que se da el agua cada vez
//bajado el 11.11.19 de 10 a 8 s
int rele        = 10; //salida a la que esta conectada el rele
int boton       = 2; //pin 2 despierta por interrupt 0
int estadoboton = HIGH;
int led         = 13; //debug

void despierta() {
  estadoboton = LOW;
}

void setup() {
  pinMode(led, OUTPUT); //debug
  pinMode(rele, OUTPUT);
  pinMode(boton, INPUT_PULLUP);
  digitalWrite(led, HIGH); //debug
  digitalWrite(rele, HIGH); //parte apagado (se prende con 0)
}

void loop() {
  attachInterrupt(0, despierta, LOW);
  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  detachInterrupt(0);

  if (estadoboton == LOW) {
    digitalWrite(led, LOW); //si esta apretado el boton, da el agua
    digitalWrite(rele, LOW); //debug
    estadoboton = HIGH;
  }
  else {
    digitalWrite(led, HIGH);
    digitalWrite(rele, HIGH); //debug
  }
  
  cuentatotal ++;

  if (cuentatotal == cuenta) {
    digitalWrite(rele, LOW); //doy el agua
    digitalWrite(led, LOW); //debug
    delay(triego * 1000); //riego
    digitalWrite(rele, HIGH); //corto el agua
    digitalWrite(led, HIGH); //debug
    cuentatotal = 0;
  }
}
