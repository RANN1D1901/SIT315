// C++ code
//
//declare the pins to be used for input or output
int sensorPin=2;
int writePin=13;
int writePinSecond=12;
int sensorPin_=1;
volatile byte r,g=0;

void setup()
{ 
  //initialise the setup with input and output pins
  pinMode(sensorPin_,INPUT);
  pinMode(sensorPin,INPUT);
  pinMode(writePin,OUTPUT);
  pinMode(writePinSecond,OUTPUT);
  
  //turn on all the ports
  PCICR |= 0b00000111;
  //turn on pins for the output on port d
  PCMSK2 |=0b10000100;
  Serial.begin(9600);
}

void loop()
{
  if(g>0 || r>0)
  {
    Serial.println("Interruption");
  }
  //write on the pins when interrupt is detected
  //green is 13 and red is 12
  digitalWrite(writePin,g);
  digitalWrite(writePinSecond,r);
}
//looking for interru
ISR(PCINT2_vect){
  g = PIND & B00000100;//pin 2  on port B
  r = PIND & B10000000;//pin 7 on port B
}