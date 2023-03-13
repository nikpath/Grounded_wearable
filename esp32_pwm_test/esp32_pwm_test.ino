#include <analogWrite.h>
#include <Arduino.h>

double frequency = 20000;

void setup(){
  Serial.begin(115200);

}
 
void loop(){
  Serial.println("hello");
analogWriteFrequency(22,frequency); //start
        analogWrite(22, 1000, 1023);
        delay(2000);  // delay one second
        
        analogWriteFrequency(22,frequency); //stop
        analogWrite(22, 0, 1023);
  delay(3000);
 
}
