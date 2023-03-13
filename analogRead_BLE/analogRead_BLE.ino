#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
#include <analogWrite.h>
#include <Arduino.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic_HR = NULL;

double frequency = 20000;
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 39; //Analog Input. 
uint32_t HRSignal; // Analog signal to read
//report a sample every 100 milliseconds (50 samples) [10 Hz]
byte samplesUntilReport;  
const byte SAMPLES_PER_SERIAL_SAMPLE = 50; 
PulseSensorPlayground pulseSensor;  //pulseSenor instance
//GSR reading
const int GSR=36;
int EDAsensorValue;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_HR "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("advertising again");
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


void setup() {
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector
//  analogWriteFrequency(22,frequency); //stop
//        analogWrite(22, 0, 1023);
//  s      
  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);
  // Skip the first SAMPLES_PER_SERIAL_SAMPLE in the loop().
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;

  // Create the BLE Device
  BLEDevice::init("Grounded_wearable_1");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics
  pCharacteristic_HR = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_HR,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY 
                    );
pCharacteristic_HR->addDescriptor(new BLE2902());
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

}

void loop() {
  Serial.println("in loop");
  analogWriteFrequency(22,frequency); //start
        analogWrite(22, 266, 1023);
        delay(2000);  // delay one second
        
        analogWriteFrequency(22,frequency); //stop
        analogWrite(22, 0, 1023);
  delay(3000);
  // read new samples
//  if (pulseSensor.sawNewSample()) {
//    if (--samplesUntilReport == (byte) 0) {
//      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
//      HRSignal = analogRead(PULSE_INPUT);
//      pCharacteristic_HR->setValue((uint8_t*)&HRSignal, 4);
//            pCharacteristic_HR->notify();
//      //EDAsensorValue=analogRead(GSR);
//      Serial.println(HRSignal);
//      //Serial.println(EDAsensorValue);
//    }
//  }
}
