#define OE   10 //6
#define A    7
#define B    8
#define CLK  9
#define LAT 6//10
#define R    11

void setup() {
  pinMode(OE, OUTPUT);
  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(CLK, OUTPUT);
  pinMode(LAT, OUTPUT);
  pinMode(R, OUTPUT);

  digitalWrite(OE, HIGH);   // salida apagada al inicio
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);
  digitalWrite(CLK, LOW);
  digitalWrite(LAT, LOW);
  digitalWrite(R, LOW);
}

void loop() {
  digitalWrite(A, LOW);
  digitalWrite(B, LOW);

  // Enviar 64 bits: solo 1 bit central en HIGH
  digitalWrite(OE, HIGH); // apagar mientras shift
  for(int i=0;i<64;i++){
    digitalWrite(R, (i==32)?HIGH:LOW);
    digitalWrite(CLK,HIGH);
    digitalWrite(CLK,LOW);
  }

  // Latch
  digitalWrite(LAT,HIGH);
  digitalWrite(LAT,LOW);

  // Activar salida
  digitalWrite(OE,LOW);
  delay(5);
  digitalWrite(OE,HIGH);

  delay(100);
}
