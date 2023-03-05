#include <analogWrite.h>
#include <Arduino.h>

double frequency = 20000;

void setup(){
}
 
void loop(){
  Serial.println("hello");
analogWriteFrequency(15,frequency); //start
        analogWrite(15, 266, 1023);
        delay(2000);  // delay one second
        
        analogWriteFrequency(15,frequency); //stop
        analogWrite(15, 0, 1023);
  delay(3000);
 
}
