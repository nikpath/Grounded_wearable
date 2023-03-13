#include <analogWrite.h>
#include <Arduino.h>

double frequency = 20000;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  analogWriteFrequency(15,frequency); //start
  analogWrite(15, 266, 1023);
  delay(1500);  // delay one second
  
  analogWriteFrequency(15,frequency); //stop
  analogWrite(15, 0, 1023);
  delay(2000); //wait 50 seconds.

}
