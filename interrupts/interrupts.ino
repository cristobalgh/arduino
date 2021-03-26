#define LED 13
#define LED2 10

unsigned long previousMillis = 0;
volatile byte state = LOW;
volatile byte state2 = LOW;
int contint = 0;
int milisactuales = 0;
int retardo = 5000;

void setup() {
  Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(LED2, OUTPUT);
  attachInterrupt(0, toggle, RISING);
}

void loop() {// rutina normal
  digitalWrite(LED2, state2);
  state2 = !state2; 
  delay(int(retardo)); 
}

void toggle() {// rutina de la interrupcion
  state = !state; 
  contint = ++contint;
  digitalWrite(LED, state);
  Serial.print("interrupcion #");
  Serial.println(contint);
  Serial.print("retardo: ");
  Serial.println(retardo);
  Serial.print("ms desde ultima interrupcion: ");
  Serial.println(int(millis())-milisactuales);
  if((int(millis()) - milisactuales) < retardo){
    milisactuales = int(millis());
    if (retardo - 500 > 0){
      retardo = retardo - 500;
    }
  }
  else {
    retardo = retardo + 500;
    milisactuales = int(millis());
  }
}
