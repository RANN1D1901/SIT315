/*
The global variables are declared: t1_load and t1_comp variables are 
used to enable timer interrupts.
D1_state,D8_state,D9_state variables are used check the pin on ports 
where the interrupt occurs.
*/
#include <avr/interrupt.h>
const int t1_load=0;
const int t1_comp=31255;
const int led_pin=PB5;
bool D8_state = LOW;
bool D9_state = LOW;
bool D1_state = LOW;
/*
The setup would be to enable the ports to read interrupts.
In this case, the interruptions are being read from PORT B and PORT D.
Three sensors are used, 2 are attached with Port B wherease one sensor with Port D.
To act on the interrupts, PortB and PortD are enables for the output stream of data,
specific pins are D10 and D2.

The next step is to enable the timer interrupts, PORT is B and the pin is D13, where timer interrupt occurs.
Prescaler is set to 1024 which enables the interrupt to occur after certain period of time 
calculated as 1/(16000000/((31255+1)/1024)) seconds.

The timer interrupt is enabled at pin D13 of the arduino, the action happening here is that it toggles the light.
*/

void setup()
{
  cli();
  //turn on all the ports-A,B,C although we use only B and D for interruptions.
  PCICR |= 0b00000111;	
  //turn on pins D0 and D1 for the input on port d
  PCMSK2 |= 0b00000011;
  //turn on pins D8 and D9 for input on port B
  PCMSK0 |= 0b00000011;
  //output pins for led to act on the interrupt from PORT B
  DDRB |=0b00000100;
  PORTB |= 0b00000100;
  //output pins for led to act on the interrupt from PORT D
  DDRD |=0b00000100;
  PORTD |= 0b00000100;
  //output for timer interrupt 
  DDRB|=(1<<led_pin);
  //reset time1 control 
  TCCR1A=0;


  //set prescaler to 1024
  TCCR1B|=(1<<CS12);
  TCCR1B&=~(1<<CS11);
  TCCR1B|=(1<<CS10);


  //reset timers 
  TCNT1=t1_load;
  OCR1A=t1_comp;
  
  //enable time1 interrupt
  TIMSK1=(1<<OCIE1A);
  sei();
}
void loop()
{
	//something happening here
}
//interrupts for PORT D
/*
ISR reads the register for PORT D to read any interrupts.
If interrupts occurs, it first toggles the led on port D, pin 10.
Second course of action is that it reads the actual pin number from,
which interrupt arrives.
Note: The Pin number is not logged on Serial monitor as it is not a good 
practice to perform a lot of instruction inside a routine function.
*/
ISR(PCINT2_vect){
  
  PORTD ^=0b00000100;  
  if(digitalRead(1) && D1_state){
   	D1_state = HIGH;
	//Pin D1 triggered the ISR
   }
   else if(digitalRead(1) && !D1_state){
   	D1_state = LOW;
    //Pin D1 triggered the ISR
   }
}
//interrupts for PORT B 
/*
ISR reads the register for PORT B to read any interrupts.
If interrupts occurs, it first toggles the led on port B, pin 2.
Second course of action is that it reads the actual pin number from,
which interrupt arrives.
Note: The Pin number is not logged on Serial monitor as it is not a good 
practice to perform a lot of instruction inside a routine function.
*/
ISR(PCINT0_vect){
  //toggle the led light
  PORTB ^=0b00000100;
  if(digitalRead(8) && D8_state){
   	D8_state = HIGH;
	//Pin D8 triggered the ISR
   }
   else if(digitalRead(8) && !D8_state){
   	D8_state = LOW;
    //Pin D8 triggered the ISR
   }
   if(digitalRead(9) && D9_state){
   	D9_state = HIGH;
	//Pin D9 triggered the ISR
   }
   else if(digitalRead(9) && !D9_state){
   	D9_state = LOW;
     //Pin D9 triggered the ISR
   }
  
}
//timer interrupts
/*
This routine function reads for instructions on the timer register of
port D, and when the interrupt is read, this function toggles the led as well
throughout the program execution.
*/
ISR(TIMER1_COMPA_vect){
  TCNT1=t1_load;
  //tohggle the led
  PORTB^=(1<<led_pin);
  
}