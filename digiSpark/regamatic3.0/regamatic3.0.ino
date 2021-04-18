/* interrupcion de timer y de boton en pin 2, esta version
   cierra el rele con un HIGH, en attiny85 led de placa
   en pin 1 en otros 13 normalmente, delay no funciona
   bien en el attiny85 da 4 veces mas tiempo */

#include "tinysnore.h" //para usar snore en vez de delay

int periodo     = 180; //m, cada cuanto quiero que riegue
int triego      = 15; //s, tiempo que se da el agua cada vez

int cuentatotal = 0;
int cuentat1    = 0;
int multiplo    = 4;
float cuenta    = round(periodo * 60);
int rele        = 0; //salida a la que esta conectada el rele
int boton       = 2; //pin 2 despierta por interrupt 0
int estadoboton = HIGH; //pullup a 5V
int led         = 1; //debug

void setup() {
  pinMode(led, OUTPUT); //debug
  pinMode(rele, OUTPUT);
  pinMode(boton, INPUT_PULLUP); //pulsador entre GND y boton
  cortaelagua();
  attachInterrupt(0, despierta, FALLING);
  setupt1();
}

void loop() {
  if (estadoboton == LOW) {
    daelagua();
    snore(triego * 1000); //riego
    cortaelagua();
    estadoboton = HIGH;
  }
  if (cuentatotal == cuenta) {
    daelagua();
    snore(triego * 1000); //riego
    cortaelagua();
    cuentatotal = 0;
  }
}

void daelagua() {
  digitalWrite(led, HIGH);
  digitalWrite(rele, HIGH);
}

void cortaelagua() {
  digitalWrite(led, LOW);
  digitalWrite(rele, LOW);
}

void despierta() {
  estadoboton = LOW;
}

ISR(TIMER1_COMPA_vect) { //entra aca cada 250,236 ms
  cuentat1++;
  if (cuentat1 == multiplo) {//cada 1s aumenta cuentatotal
    cuentatotal++;
    cuentat1 = 0;
  }
}

void setupt1() {
  cli();
  TCCR1 = 0;
  TCNT1 = 0;              // zero the timer
  GTCCR = _BV(PSR1);      // reset the prescaler
  OCR1A = 251;
  // contador CTC, tiene un tamaño maximo de 2^8-1=255
  OCR1C = 251;
  TCCR1 |= (1 << CTC1);   // el analogo a WGM12 en los atmega
  TCCR1 |= (1 << CS10);
  TCCR1 |= (1 << CS11);
  TCCR1 |= (1 << CS12);
  TCCR1 |= (1 << CS13);   // CS10=1, CS11=1, CS12=1, CS13=1
  // ==> prescaler=16384 (ver datasheet attiny85)
  // luego T=1/(f/prescaler)==> 16384/16.5MHz = 993us
  // 993us*CTC=993us*252= 250,236 ms
  TIMSK = (1 << OCIE1A);
  // para habilitar la comparacion Output Compare
  // A Match (vector interrupcion)
  sei();
}
