  int switchPin = 3 ;
int ledPin = 13 ;
boolean lastButton = LOW;
boolean currentButton = LOW;
boolean Started = false;
boolean timer = false;
long startTime;
long endTime;
int randomTime;
long beginTime;
float elapsedTime;

void setup()
{
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  Serial.begin(9600);
}
boolean debounce(boolean last)
{
  boolean current = digitalRead(switchPin);
  if(last != current)
  {
    delay(5);
    current = digitalRead(switchPin);
  }
  return current;
}

void loop()
{
  // see if button pressed
  currentButton = debounce(lastButton);
  if(lastButton == LOW && currentButton == HIGH)
  {
    if(Started==false){
      Started=true;
      randomTime = random(4,10);
      randomTime = randomTime*1000;
      Blink();
      beginTime=millis(); 
      
    }
    else{
      if((millis()-beginTime)>=randomTime){
          Stop();
          Started=false; 
          timer=false;         
      }
      
      else{
        Started=false;
        timer=false;
        Serial.println("Apretaste el boton antes!");
        for(int i=0; i<3; i++){
          Blink();
        }
      }
    }
  }
     
  lastButton = currentButton;
  
  if(Started == true && (millis()-beginTime)>=randomTime && timer==false){
    Serial.println("Inicio");
    timer=true;
    Start();
  }
}

void Start(){
  startTime = millis();
  digitalWrite(ledPin, HIGH);
}

void Blink(){
  digitalWrite(ledPin, HIGH);
  delay(100);
  digitalWrite(ledPin, LOW);
  delay(100);
}

void Stop(){
  endTime = millis();
  elapsedTime = (endTime - startTime)+5;
  elapsedTime = elapsedTime;
  Serial.print("Tiempo en milisegundos: ");
  Serial.println(int(elapsedTime));
  digitalWrite(ledPin, LOW);  
}
