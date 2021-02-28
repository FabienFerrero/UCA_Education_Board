/*
 Controlling a servo position 
Connect :
- brown cable to GND
- Red Cable to V_in (5V from USB)
- Orange to A0
 
*/

#include <Servo.h>

Servo myservo;  // create servo object to control a servo

int potpin = 0;  // analog pin used to connect the potentiometer
int val;    // variable to read the value from the analog pin

void setup() {
  myservo.attach(A2);  // attaches the servo on pin 9 to the servo object
}

void loop() {

  for (int i=0; i<=180; i=i+5){
  //val = analogRead(potpin);            // reads the value of the potentiometer (value between 0 and 1023)
  val = map(val, 0, 1023, 0, 180);     // scale it to use it with the servo (value between 0 and 180)
  myservo.write(i);                  // sets the servo position according to the scaled value
  delay(100);                           // waits for the servo to get there
  }
}
