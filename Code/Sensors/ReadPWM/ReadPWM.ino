/*
  ReadPWM

  Reads an analog input on pin A2, converts it to voltage, and prints the result to the Serial Monitor.  
  A PWM is set on PIN 5 and duty cylcle is sweep from 0 to 255
  Graphical representation is available using Serial Plotter (Tools > Serial Plotter menu).
  
*/

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {


for (int dc = 128 ; dc <= 128; dc += 5) {
    // sets the value (range from 0 to 255):
    analogWrite(5, dc);
    // wait for 30 milliseconds to see the dimming effect
    delay(100);
    
  // read the input on analog pin 0:
  int sensorValue = analogRead(A2);
  // Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
  float voltage = sensorValue * (3.3 / 1023.0);
  // print out the value you read:
  Serial.println(voltage);

     }
}
