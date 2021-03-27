/*Este codigo define dos tiempos aleatoriamente
 * on y off y con eso maneja un led en el pin 1
 */
#include "tinysnore.h" //consumo de hasta 0.0047 mA
#define A 1

int on, off;
int mini  = 20; //ms
int maxi  = 2000; //ms

void setup(){
  pinMode(A, OUTPUT); 
}

// the loop routine runs over and over again forever:
void loop(){
  on  = random(mini,maxi);
  digitalWrite(A, HIGH);
  snore(on);
  digitalWrite(A, LOW); 
  snore(off);
  off = random (mini,maxi);
}
