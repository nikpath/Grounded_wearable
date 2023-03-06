/*
 * Should poll both biometric sensors and send back BPM, IBI and EDA values every 2 min
 * See BLE_mock for how the BLE should work
*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>

#include <analogWrite.h>
#include <Arduino.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic_HR = NULL;
BLECharacteristic* pCharacteristic_EDA = NULL;
BLECharacteristic* pCharacteristic_PAUSE = NULL;
BLECharacteristic* pCharacteristic_BLS = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;
bool pausePolling = false;
double frequency = 20000;
bool BLS_ON = false;

/*
 * Pulse sensor constants
*/
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 35;
/*
 * EDA sensor constants
*/
const int GSR_INPUT=34;

uint32_t HR = 0;
uint32_t EDA = 0;

byte samplesUntilReport;
const byte SAMPLES_PER_SERIAL_SAMPLE = 50; 

/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_HR "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_EDA "58260ca5-a468-496a-8f8c-ad30a21ba7cf"
#define CHARACTERISTIC_UUID_PAUSE "885bccf9-007a-4050-aa92-a9da38199deb"
#define CHARACTERISTIC_UUID_BLS "59613ebd-37e4-4c49-8f2d-a4d440207607"


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

class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++){
          Serial.print(value[i]);
          if (value[i] == 1){ //pause polling
            pausePolling = true;
          } else if (value[i] == 2){ //unpause polling
            pausePolling = false;
            } else if (value[i] == '3') { //dont pause polling and start bls
            Serial.print("bls");
            pausePolling = true;
            BLS_ON = true;
          } else if (value[i] == '4') { //pause BLS start polling
            Serial.print("unpaused and no bls");
            pausePolling = false;
            BLS_ON = false;
            
          }
        }

        Serial.println();
        Serial.println("*********");
      }
  }
};

void setup() {
  Serial.begin(115200);
  BLS_ON = false;

  analogWriteFrequency(15,frequency); //stop
  analogWrite(15, 0, 1023);

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
  pCharacteristic_PAUSE = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_PAUSE,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE
                    );
  pCharacteristic_BLS = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_BLS,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

pCharacteristic_PAUSE->setCallbacks(new MyCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create BLE Descriptors
  pCharacteristic_HR->addDescriptor(new BLE2902());
  pCharacteristic_EDA->addDescriptor(new BLE2902());
  pCharacteristic_PAUSE->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");

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
      // notify changed value
    if (deviceConnected) {
      if(!pausePolling) {
        if (pulseSensor.sawNewSample()) {
          if (--samplesUntilReport == (byte) 0) {
            Serial.println("POLLING ON");
            samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;
            HR = analogRead(PULSE_INPUT);
            EDA=analogRead(GSR_INPUT);

            pCharacteristic_HR->setValue((uint8_t*)&HR, 4);
            pCharacteristic_EDA->setValue((uint8_t*)&EDA, 4);
            pCharacteristic_HR->notify();
            pCharacteristic_EDA->notify();
        
          }
         }
      } else if(BLS_ON) {
        Serial.println("BLS ON 1");
        analogWriteFrequency(15,frequency); //start
        analogWrite(15, 266, 1023);
        delay(1000);  // delay one second

        
        pCharacteristic_BLS->setValue("y");
        pCharacteristic_BLS->notify();
        analogWriteFrequency(15,frequency); //stop
        analogWrite(15, 0, 1023);
        delay(3000); //wait 50 seconds.
        
        } else {
        Serial.println("waiting to resume");
        delay(2000);
      }
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        pausePolling = false;
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        pausePolling = false;
        oldDeviceConnected = deviceConnected;
    }
    
}
