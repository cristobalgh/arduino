#include <Servo.h> 
Servo doorServo;
Servo handServo;

int switch_pin = 2;               //set switch on pin 2

//motor variables
int motorThrottle=11;
int motorDirection = 13;
int motorBrake=8;

//Distance Variables
int motionPin = 0;    //motion sensor pin on analog 0
int lastDist = 0;      //to remember last distance
int currentDist = 0;   
int thresh = 100;      //Threshold for Movement


int pos = 0;
int selectedMove = 0;             //move selector
int Testmove = 0;                 //test mode: set to move number to test only one selected move 
                                  //(set to Zero to run normally i.e: roundrobbin on amm moves)



void setup()
{
  Serial.begin(9600);
  pinMode(switch_pin, INPUT);
  doorServo.attach(9);           //set door servo on Pin 9 pwm
  handServo.attach(10);          //set hand servo on Pin 10 pwm
  doorServo.write(80);           //set door to hiding position 
  handServo.write(0);            //set hand to hiding position
  //Setup Channel B, since Channel A is reserved by door and hand servos and can't ber used at same time
  pinMode(motorDirection, OUTPUT); //Initiates Motor Channel B pin
  pinMode(motorBrake, OUTPUT); //Initiates Brake Channel B pin
  
}


void loop()
{
if (Testmove != 0) {
selectedMove = Testmove;
}
  
  //if the switch is on, then move door and hand to switch it off...
  if(digitalRead(switch_pin) == HIGH)
  {
    
  if (selectedMove > 10) { selectedMove = 0; } //when all moves are played, repeat the moves from beginning 
  
  if (selectedMove == 0) { switchoff(); }
  else if (selectedMove == 1) { switchoff(); }   
  else if (selectedMove == 2) { switchoffbitaraddod(); }
  else if (selectedMove == 3) { crazydoor(); }
  else if (selectedMove == 4) { crazyslow(); }
  else if (selectedMove == 5) { m7anika7anika(); }
  else if (selectedMove == 6) { m3alla2(); }
  else if (selectedMove == 7) { switchoff(); }
  else if (selectedMove == 8) { matrix(); }
  else if (selectedMove == 9) { sneak(); }
  else if (selectedMove == 10) { zee7(); }
   
  if (Testmove == 0) {
  selectedMove++;         //swith to next move if not in test mode
  }

}
}



// Library of moves

   // basic move 
   void switchoff() 
   {    
   //Moving door
    for(pos = 80; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
   
    //Moving hand
    for(pos = 0; pos < 129; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }  
    
    //hiding hand
    for(pos = 129; pos>=0; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    } 
      
    //hiding door
    for(pos = 155; pos>=80; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 
   } 
     
   // move 3: open and wait, then move hand and wait, then switch of and hide  
    void switchoffbitaraddod()
   {
  //Moving door
    for(pos = 80; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
    delay(800); 
    //Moving hand
    for(pos = 0; pos < 100; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }
    delay(1000); 
    for(pos = 100; pos < 129; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }    
    
    //hiding hand
    for(pos = 129; pos>=0; pos-=5)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    } 
      
    //hiding door
    for(pos = 155; pos>=80; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);                    
     
     
   } 
     
   }
  
   //move 4: open door then close it many times, wait, then quickly reoprn a nd switch off and hide.
  
   void crazydoor()
  {
    
   //Moving door
    for(pos = 80; pos < 125; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
  
    //hiding door
    for(pos = 125; pos>=80; pos-=5)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 
   //Moving door
    for(pos = 80; pos < 110; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
  
    //hiding door
    for(pos = 110; pos>=80; pos-=15)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 
    delay(700);
   //Moving door
    for(pos = 80; pos < 125; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
    delay(700);
    //hiding door
    for(pos = 125; pos>=80; pos-=5)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 

    
    //----of switch of----//
  //Moving door
    for(pos = 80; pos < 155; pos += 8)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
   
    //Moving hand
    for(pos = 0; pos < 129; pos += 3)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }  
    
    //hiding hand
    for(pos = 129; pos>=0; pos-=3)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    } 
      
    //hiding door
    for(pos = 155; pos>=80; pos-=15)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    }    
    
  }   


 // move 5: open door, then move hand very slowly forward and back to hiding very slowly, then quickly close door
 void crazyslow()
 {
 
//Moving door
    for(pos = 80; pos < 155; pos += 1)   
    {                                   
    doorServo.write(pos);              
    delay(30);                       
    }
   
    //Moving hand
    for(pos = 0; pos < 129; pos += 1)  
    {                                   
    handServo.write(pos);               
    delay(30);                       
    }  
    
    //hiding hand
    for(pos = 129; pos>=0; pos-=1)      
    {                                
    handServo.write(pos);               
    delay(30);                        
    } 
      
    //hiding door
    for(pos = 155; pos>=125; pos-=1)     
    {                                
    doorServo.write(pos);              
    delay(30);                      
    }
    delay(100);
    for(pos = 125; pos>=80; pos-=4)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    }     
    
 
 }
 
 //move 6:
 
 void m7anika7anika() {
 
//Moving door
    for(pos = 80; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
     
    //Moving hand
    for(pos = 0; pos < 70; pos += 1)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }
    delay(800);
    
    
    //hiding door
    for(pos = 155; pos>=130; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15); 
    }
    
    //hiding door
    for(pos = 130; pos < 155; pos+=3)     
    {                                
    doorServo.write(pos);              
    delay(15); 
    } 
     //hiding door
    for(pos = 155; pos>=130; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15); 
    }   
    //hiding door
    for(pos = 130; pos < 155; pos+=3)     
    {                                
    doorServo.write(pos);              
    delay(15); 
    } 
     
    handServo.write(40);
    delay(1000);
      
    //Moving hand
    for(pos = 40; pos < 129; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    } 
    
    //hiding hand
    for(pos = 129; pos>=0; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    } 
    
      
    for(pos = 120; pos>=80; pos -= 1)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
   
   
     
} 

void m3alla2(){
//Moving door
    for(pos = 80; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
   
    //Moving hand
    for(pos = 0; pos < 127; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }
    //hiding door
    for(pos = 155; pos>=130; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    }   
    delay(2000);
    
    for(pos = 130; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
    
    for(pos = 155; pos>=140; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);
    }
    for(pos = 140; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
    delay(500);
    //hiding hand
    for(pos = 127; pos>=0; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    } 
      
    //hiding door
    for(pos = 155; pos>=80; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 

}
   
void matrix()
{

 //Moving door
    for(pos = 80; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
   
    //Moving hand
    for(pos = 0; pos < 80; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }
    
    for(pos = 80; pos < 129; pos += 1)  
    {                                   
    handServo.write(pos);               
    delay(30);                       
    }  
    delay(300);
    
    for(pos = 129; pos>=0; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(10);                        
    } 
      
    //hiding door
    for(pos = 155; pos>=80; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 
  
}

void sneak() 
   {    
   //Moving door
    for(pos = 80; pos < 130; pos += 1)   
    {                                   
    doorServo.write(pos);              
    delay(30);                       
    }
    delay(2000);
    
    //Moving hand
    for(pos = 0; pos < 40; pos += 1)  
    {                                   
    handServo.write(pos);               
    delay(30);                       
    }  
        
    delay(500);
    
    for(pos = 130; pos < 155; pos += 4)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
    delay(100);
    
    for(pos = 40; pos < 90; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }  
    delay(500);
    //hiding hand
    for(pos = 90; pos>=70; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    }
    delay(100);
    for(pos = 70; pos < 90; pos += 4)  
    {                                   

    handServo.write(pos);               
    delay(15);                       
    }
    delay(100);
    for(pos = 90; pos>=70; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    }
    delay(100);
       
    for(pos = 70; pos < 129; pos += 4)  
    {                                   

    handServo.write(pos);               
    delay(15);                       
    }
    
    for(pos = 129; pos>=0; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    }    
    //hiding door
    for(pos = 155; pos>=80; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 
   } 
   
   
   void zee7() 
   {    
   //Moving door
    for(pos = 80; pos < 155; pos += 3)   
    {                                   
    doorServo.write(pos);              
    delay(15);                       
    }
   delay(2000);
   //forward @ full speed to the left
  digitalWrite(motorDirection, LOW); //Establishes RIGHT direction of Channel A
  digitalWrite(motorBrake, LOW);      //Disengage the Brake for Channel B
  analogWrite(motorThrottle, 100);    //Spins the motor on Channel B at full speed
  delay(300);
  digitalWrite(motorBrake, HIGH); //Eengage the Brake for Channel B
  delay(2000);
  
  //backward @ full speed to the right
  digitalWrite(motorDirection, HIGH); //Establishes LEFT direction of Channel B
  digitalWrite(motorBrake, LOW);   //Disengage the Brake for Channel B
  analogWrite(motorThrottle, 100);   //Spins the motor on Channel B at half speed
  delay(300);
  digitalWrite(motorBrake, HIGH); //Eengage the Brake for Channel B
  delay(1000);
  
    //Moving hand
    for(pos = 0; pos < 129; pos += 4)  
    {                                   
    handServo.write(pos);               
    delay(15);                       
    }  
    
    //hiding hand
    for(pos = 129; pos>=0; pos-=4)      
    {                                
    handServo.write(pos);               
    delay(15);                        
    } 
      
    //hiding door
    for(pos = 155; pos>=80; pos-=3)     
    {                                
    doorServo.write(pos);              
    delay(15);                      
    } 
   } 
