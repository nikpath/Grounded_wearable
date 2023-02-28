#include <analogWrite.h>
#include <Arduino.h>

double frequency = 20000;

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  analogWriteFrequency(15,frequency);
  analogWrite(15, 266, 1023);

}
