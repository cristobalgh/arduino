//Config Timer1 en attiny85 constantes y vars
#define led 1
#define multiplo 4        // con 4 interrupciones es 1s
volatile int cuenta = 0;

void setup() {
  pinMode(led, OUTPUT);
  cli();
  TCCR1 = 0;
  TCNT1 = 0;              // zero the timer
  GTCCR = _BV(PSR1);      // reset the prescaler
  OCR1A = 251;            // contador CTC, tiene un tamaño maximo de 2^8-1=255
  OCR1C = 251;
  TCCR1 |= (1 << CTC1);   // el analogo a WGM12 en los atmega
  TCCR1 |= (1 << CS10);
  TCCR1 |= (1 << CS11);
  TCCR1 |= (1 << CS12);
  TCCR1 |= (1 << CS13);   // CS10=1, CS11=1, CS12=1, CS13=1
                          // ==> prescaler=16384 (ver datasheet attiny85)
                          // luego T=1/(f/prescaler)==> 16384/16.5MHz = 993us
                          // 993us*CTC=993us*252= 250 ms
  TIMSK = (1 << OCIE1A);  // para habilitar la comparacion Output Compare
                          // A Match (vector interrupcion)
  sei();
}

void loop(){
}

//rutina interrupcion
ISR(TIMER1_COMPA_vect){ //entra aca cada 242 ms
  cuenta++;
  if (cuenta>=multiplo){
    digitalWrite(led, !digitalRead(led));
    cuenta = 0;
    }
}
