//Includes
#include <avr/io.h>
#include <avr/interrupt.h>

#define INTERRUPTPIN PCINT1       //this is PB1 per the schematic
#define PCINT_VECTOR PCINT0_vect  //this step is not necessary
#define DATADIRECTIONPIN DDB1     //Page 64 of data sheet
#define PORTPIN PB1               //Page 64
#define READPIN PINB1             //page 64
#define LEDPIN 4                  //PB4
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit)) //OR
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit)) //AND
/*
   Alias for the ISR: "PCINT_VECTOR" (Note: There is only one PCINT ISR.
   PCINT0 in the name for the ISR was confusing to me at first,
   hence the Alias, but it's how the datasheet refers to it)
*/
static volatile byte LEDState; //variable used within ISR must be declared Volatile.

void setup(){
  cli();                            //disable interrupts during setup
  pinMode(LEDPIN, OUTPUT);          //we can use standard arduino style for this as an example
  digitalWrite(LEDPIN, LOW);        //set the LED to LOW
  LEDState = 0;                     //we use 0 for Low state and 1 for High
  PCMSK |= (1 << INTERRUPTPIN);     //sbi(PCMSK,INTERRUPTPIN) also works but I think this is more clear
                                    // tell pin change mask to listen to pin2 /pb3 //SBI
  GIMSK |= (1 << PCIE);             //enable PCINT interrupt in the general interrupt mask //SBI
  DDRB &= ~(1 << DATADIRECTIONPIN); //cbi(DDRB, DATADIRECTIONPIN)
                                    //set up as input  - pin2 clear bit  - set to zero
  PORTB |= (1 << PORTPIN);          //cbi(PORTB, PORTPIN)
                                    //disable pull-up. hook up pulldown resistor. - set to zero
  sei();                            //last line of setup - enable interrupts after setup
}

void loop(){
  //put your main code here, to run repeatedly:
  //If you connect a debounced pushbutton to PB2 (PB1?)
  //and to VCC you can tap the button and the LED will come on
  //tap the button again and the LED will turn off.
}

//this is the interrupt handler
ISR(PCINT_VECTOR){
  //Since the PCINTn triggers on both rising and
  //falling edge let's just looks for rising edge
  //i.e. pin goes to 5v
  byte pinState;
  pinState = (PINB >> READPIN) & 1; //PINB is the register to read the state of the pins
  if (pinState > 0){                //look at the pin state on the pin
                                    //PINB register- returns 1 if high
    if (LEDState == 0){
      digitalWrite(LEDPIN, HIGH);   //you can use Arduino Code
                                    //or LowerLevel Code to write to the register
      LEDState = 1;
    }
    else{
      digitalWrite(LEDPIN, LOW);
      LEDState = 0;
    }
  }
}
