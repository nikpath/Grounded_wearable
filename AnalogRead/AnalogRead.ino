#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 39;
int HRSignal; // Analog signal to read

//report a sample every 100 milliseconds (50 samples) [10 Hz]
byte samplesUntilReport;  
const byte SAMPLES_PER_SERIAL_SAMPLE = 50; 

PulseSensorPlayground pulseSensor;

//GSR reading
const int GSR=36;
int EDAsensorValue;

void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);

    // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  //pulseSensor.setThreshold(PULSE_THRESHOLD);
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    Serial.println("Pulse sensor initialization failed");
  }
  
  //set the resolution to 12 bits (0-4096)
  //sanalogReadResolution(12);
}

void loop() {
  if (pulseSensor.sawNewSample()) {
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      HRSignal = analogRead(PULSE_INPUT);
      EDAsensorValue=analogRead(GSR);
      Serial.println("HR: ");
      Serial.println(HRSignal);
      Serial.println("EDA: ");
      Serial.println(EDAsensorValue);
    }
  }
}
