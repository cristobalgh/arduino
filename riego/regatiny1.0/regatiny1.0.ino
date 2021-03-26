//esta version cierra el rele con un HIGH no como el rele
//que viene con placa y que cierra con un LOW

#include "tinysnore.h"
#define rele  0           //en este pin se conecta el rele
#define led   1           //led de la placa conectada a este pin

int triego    = 15000;    //ms prendido 15s prendido
int toff      = 10785000; //ms apagado 10785s apagado, 3h - 15s
int encendido = 1;        //se acaba de enchufar

void setup(){
  pinMode(led, OUTPUT);   //para debugging
  pinMode(rele, OUTPUT);
  cortaelagua();
}

void loop() {
  if (encendido == 1) {   //cuando enchufo se prende altiro
    daelagua();
    snore(triego);
    cortaelagua();
    encendido = 0;
  }
  snore(toff);
  daelagua();
  snore(triego);
  cortaelagua();
}

void daelagua() {
  digitalWrite(led, HIGH);
  digitalWrite(rele, HIGH);
}
void cortaelagua() {
  digitalWrite(led, LOW);
  digitalWrite(rele, LOW);
}
