// C++ code
/*
The setup functions sets up the input pins and output pins using PCINT library.
The led's are setup for output on PortB, pins 12 and 13 with intial setup as being swithced off.
The input ports are B and D with pins being 6 and 2 on arduino.
When interrupt occurs at portD, i.e., pin2, the led on pin 13 is toggled.
When interrupt occurs at portB, i.e., pin3, the led on pin 12 is toggled.
The led is toggled within ISR function and nothing happens in loop().
*/
void setup()
{ 
  /*
  Output stream is enabled for port B output pins and default state of all pins is LOW.
  */
  DDRB |=0b00110000;
  PORTB |=(0b00000000);
  /*
  Input setup for interrupt is happening within this section of code.
  All ports are enabled.
  Pin 2 of Port D is enabled.
  Pin 8 of Port B is enables.
  */
  //turn on all the ports
  PCICR |= 0b00000111;
  //turn on pins for the output on port d:pin2
  PCMSK2 |=0b10000100;
  //turn on pins for the output on port b:pin 8
  PCMSK0 |=0b00000001;

  Serial.begin(9600);
}
void loop(){
  //let's assume some event occurs here.
}

//looking for interrupts at port d 
/*
This function reads the register for Port D for any changes and toggles the led.
*/
ISR(PCINT2_vect){
  Serial.println("Interrupt at port d");
  PORTB ^=(0b00100000);  
}
//looking for interrupts at port b
/*
This function reads the register for Port B for any changes and toggles the led.
*/
ISR(PCINT0_vect){
    Serial.println("Interrupt at port b");
    PORTB ^=(0b00010000);  
}

