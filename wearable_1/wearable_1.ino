/*
 * Wearable 1: Starts a BLE server (clients are phone app and second wearable)
 * Once connected, it continously polls the sensors and sends this data via BLE to phone app
 * If BLS is turned on, vibration motors will start in an alternating rhythm whilst polling continues
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include <analogWrite.h>
#include <Arduino.h>

/*
 * General global constants
*/
uint8_t deviceConnected = 0;
bool oldDeviceConnected = false;
bool pausePolling = false; //if true, biometric sensor polling is off
double frequency = 20000; //vibration motor frequency
bool BLS_ON = false; //if true, motors are doing BLS
bool BLS_MOTOR_ON = false;
byte samplesUntilReport; //biometric sampling rate
const byte SAMPLES_PER_SERIAL_SAMPLE = 50; 

/*
 * Pulse sensor constants
*/
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 39;
uint32_t HR = 0;
PulseSensorPlayground pulseSensor;

/*
 * EDA sensor constants
*/
const int GSR_INPUT=36;
uint32_t EDA = 0;

/*
 * BLS Timer constants
*/
volatile int interruptCounter;
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
void IRAM_ATTR onTimer() {
  portENTER_CRITICAL_ISR(&timerMux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&timerMux);
}

/*
 * BLE constants
*/
BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic_HR = NULL; //pulse sensor data
BLECharacteristic* pCharacteristic_EDA = NULL; //GSR sensor data
BLECharacteristic* pCharacteristic_CONTROL = NULL; //comms between app and wearable 1 for pausing polling, bls and device connection status
BLECharacteristic* pCharacteristic_BLS = NULL; //notifies second wearable when to turn on bls

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_HR "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_EDA "58260ca5-a468-496a-8f8c-ad30a21ba7cf"
#define CHARACTERISTIC_UUID_CONTROL "885bccf9-007a-4050-aa92-a9da38199deb"
#define CHARACTERISTIC_UUID_BLS "59613ebd-37e4-4c49-8f2d-a4d440207607"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected++;
      pCharacteristic_CONTROL->setValue((uint8_t*)&deviceConnected, 4);
      pCharacteristic_CONTROL->notify();
      if(deviceConnected < 2) { //continue advertising since need to be connected to 2 devices
        Serial.println("callback advertising");
        BLEDevice::startAdvertising();
      }
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected--;
      pCharacteristic_CONTROL->setValue((uint8_t*)&deviceConnected, 4);
      pCharacteristic_CONTROL->notify();
      BLEDevice::startAdvertising();
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
        for (int i = 0; i < value.length(); i++){
          if (value[i] == '3'){ //pause polling
            Serial.println("PAUSED POLLING");
            pausePolling = true;
          } else if (value[i] == '4'){ //unpause polling
            Serial.println("UNPAUSED POLLING");
            pausePolling = false;
            } else if (value[i] == '5') { //start BLS
            BLS_ON = true;
            
          } else if (value[i] == '6') { //stop BLS
            BLS_ON = false;
            analogWriteFrequency(22,frequency); //stop
            analogWrite(22, 0, 1023);
          }
        }
      }
  }
};

void pollBiometrics() {
  if (pulseSensor.sawNewSample()) {
    if (--samplesUntilReport == (byte) 0) {
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
      HR = analogRead(PULSE_INPUT);
      EDA=analogRead(GSR_INPUT);
      
      pCharacteristic_HR->setValue((uint8_t*)&HR, 4);
      pCharacteristic_EDA->setValue((uint8_t*)&EDA, 4);
      pCharacteristic_HR->notify();
      pCharacteristic_EDA->notify();
    }
  }
}


void BLS_motor_handler() { //turning motors on and off for BLS
  if(BLS_MOTOR_ON) { // turn motor off, notify other motor to turn on (on other wearable
    BLS_MOTOR_ON = false;
    pCharacteristic_BLS->setValue("y");
    pCharacteristic_BLS->notify();
    analogWriteFrequency(22,frequency); //stop
    analogWrite(22, 0, 1023);
  } else { //turn motor on
    BLS_MOTOR_ON = true;
    analogWriteFrequency(22,frequency); //start
    analogWrite(22, 500, 1023);
  }
  
}

void setup() {
  Serial.begin(115200);
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable   detector

  //keep motor off to start
  BLS_ON = false;
  BLS_MOTOR_ON = false;
  analogWriteFrequency(22,frequency); //stop
  analogWrite(22, 0, 1023);

  
  //BLS timer initialization
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 2000000, true);
  timerAlarmEnable(timer);
  
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
  pCharacteristic_EDA = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_EDA,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY 
                    );
  pCharacteristic_CONTROL = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_CONTROL,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic_BLS = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_BLS,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic_CONTROL->setCallbacks(new MyCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create BLE Descriptors
  pCharacteristic_HR->addDescriptor(new BLE2902());
  pCharacteristic_EDA->addDescriptor(new BLE2902());
  pCharacteristic_CONTROL->addDescriptor(new BLE2902());
  pCharacteristic_BLS->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();

  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);

  pulseSensor.setSerial(Serial);
  pulseSensor.setOutputType(OUTPUT_TYPE);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    Serial.println("Pulse sensor initialization failed");
  }
}

void loop() {
  if(interruptCounter > 0) {
          portENTER_CRITICAL(&timerMux);
          interruptCounter--;
          if(BLS_ON) {
            BLS_motor_handler();
          }
          portEXIT_CRITICAL(&timerMux);
          
    }
    //device connected, poll values
    if (deviceConnected >= 2) {
      if(!pausePolling) {
        pollBiometrics();
      } else {
        delay(500);
      }
    }
    
    // disconnecting
    if (deviceConnected < 2 && oldDeviceConnected) {
      Serial.println("disconnect advertising");
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        pausePolling = false;
        oldDeviceConnected = false;
    }
    
    // connecting
    if (deviceConnected >= 2 && !oldDeviceConnected) {
        // do stuff here on connecting
        pausePolling = false;
        oldDeviceConnected = true;
    }
    
}
