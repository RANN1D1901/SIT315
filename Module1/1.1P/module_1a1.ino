// C++ code
//
//declare the pins to be used for input or output
int sensorPin=2;
int writePin=5;
int writePinSecond=7;


void setup()
{ 
  //initialise the setup with input and output pins
  pinMode(sensorPin,INPUT);
  pinMode(writePin,OUTPUT);
  pinMode(writePinSecond,OUTPUT);
  Serial.begin(9600);
}
int readMotion()
{
  //detects the motion continuously on the respective pin
  Serial.println("Started to detect motion");
  int reading= digitalRead(sensorPin);
  return reading;
}
void turnOnLed()
{
  //turns the led red 
  Serial.println("Turning on the led");
  digitalWrite(writePinSecond,LOW);
  digitalWrite(writePin,HIGH);

}

void turnOffLed()
{
  //turns the led green
  Serial.println("Turning off the led");
  digitalWrite(writePin,LOW);
  digitalWrite(writePinSecond,HIGH);
}
void loop()
{
  //read motion
  int motion=readMotion();
  
  if(motion==1)
  {
    //motion detected
    turnOnLed();
  }
  else
  {
    //no motion present
    turnOffLed();
  }
}