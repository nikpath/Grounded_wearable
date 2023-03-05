#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 26;
const int PULSE_THRESHOLD = 2000; 

PulseSensorPlayground pulseSensor;

void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

    // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  //pulseSensor.setThreshold(PULSE_THRESHOLD);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    Serial.println("Pulse sensor initialization failed");
  }
  
  //set the resolution to 12 bits (0-4096)
  //sanalogReadResolution(12);
}

void loop() {
  // read the analog / millivolts value for pin 2:
  int analogValue = analogRead(PULSE_INPUT);
  //int analogVolts = analogReadMilliVolts(2);
  
  // print out the values you read:
  Serial.printf("ADC analog value = %d\n",analogValue);
  //Serial.printf("ADC millivolts value = %d\n",analogVolts);
  
  delay(1000);  // delay in between reads for clear read from serial
}
