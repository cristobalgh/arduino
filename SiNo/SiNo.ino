const int button1Pin = 2;  // pushbutton 1 pin
const int ledPinV = 12;    // LED verde pin
const int ledPinR = 13;    // LED rojo pin
const int ledPins[] = {ledPinV,ledPinR};
const int espera = 6000; // antes de apagar todo

void setup()
{
  Serial.begin(9600);
  randomSeed(analogRead(0));
  
  // Set up the pushbutton pins to be an input:
  pinMode(button1Pin, INPUT);
  //pinMode(button2Pin, INPUT);

  // Set up the LED pin to be an output:
  pinMode(ledPinR, OUTPUT);
  pinMode(ledPinV, OUTPUT);  
}

void loop()
{
  int button1State;
  button1State = digitalRead(button1Pin);
 
  if(button1State == LOW)
  {
    loopRandom();
  }
  else
  {
    digitalWrite(ledPinR, LOW);
    digitalWrite(ledPinV, LOW);
  }
}

void loopRandom()
{
  int index, i;
  index = random(25,50);
  for(i=0; i<=index; i++)
   {
     //randomLED();
    toogleLEDS(i*3);   
   }
   final();
}

void randomLED()
{
  int index, i;
  int delayTime;
  
  i=random(1000);
  if(i < 500)
  {
    index=0;
  }
  else
  {
    index=1;
  }
  //index = 1;//random(1);	// pick a random number between 0 and 1
  delayTime = 50;//random(10,25); //tiempo entre prender y apagar
  
  digitalWrite(ledPins[index], HIGH);  // turn LED on
  delay(delayTime);                    // pause to slow down
  digitalWrite(ledPins[index], LOW);   // turn LED off
  delay(delayTime);
}

int toogleLEDS(int t)
{
  //int i;
  //for(i=0;i<10;i++)
  //{
    digitalWrite(ledPins[0], HIGH);
    digitalWrite(ledPins[1], LOW);
    delay(t);
    digitalWrite(ledPins[1], HIGH);
    digitalWrite(ledPins[0], LOW);
    delay(t);
  //}
}

void final()
{
  int index, i;
  //index = random(1);
  i=random(1000);
  if(i < 500)
  {
    index=0;
  }
  else
  {
    index=1;
  }
  
  digitalWrite(ledPins[0], LOW);
  digitalWrite(ledPins[1], LOW);
  digitalWrite(ledPins[index], HIGH);
  delay(espera);
  digitalWrite(ledPins[0], LOW);
  digitalWrite(ledPins[1], LOW);
}
