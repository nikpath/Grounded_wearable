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

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic_BPM = NULL;
BLECharacteristic* pCharacteristic_IBI = NULL;
BLECharacteristic* pCharacteristic_EDA = NULL;
BLECharacteristic* pCharacteristic_PAUSE = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
bool pausePolling = false;

/*
 * Pulse sensor constants
*/
const int OUTPUT_TYPE = SERIAL_PLOTTER;
const int PULSE_INPUT = 36;
const int PULSE_THRESHOLD = 2000;   // Adjust this number to avoid noise when idle

/*
 * EDA sensor constants
*/
const int GSR=34;
const int EDA_THRESHOLD=0;

uint32_t BPM = 0;
uint32_t IBI = 0;
uint32_t EDA = 0;
/*
   All the PulseSensor Playground functions.
*/
PulseSensorPlayground pulseSensor;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_BPM "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_IBI "98260937-1924-4af9-a874-3ad204344e1e"
#define CHARACTERISTIC_UUID_EDA "58260ca5-a468-496a-8f8c-ad30a21ba7cf"
#define CHARACTERISTIC_UUID_PAUSE "885bccf9-007a-4050-aa92-a9da38199deb"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
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
          if (value[i] == 1){
            pausePolling = true;
          } else if (value[i] == 2){
            pausePolling = false;
            }
        }

        Serial.println();
        Serial.println("*********");
      }
  }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("Grounded_wearable_1");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics
  pCharacteristic_BPM = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_BPM,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY 
                    );
   
  pCharacteristic_IBI = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_IBI,
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

pCharacteristic_PAUSE->setCallbacks(new MyCallbacks());

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create BLE Descriptors
  pCharacteristic_BPM->addDescriptor(new BLE2902());
  pCharacteristic_IBI->addDescriptor(new BLE2902());
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
  pulseSensor.setThreshold(PULSE_THRESHOLD);

  // Now that everything is ready, start reading the PulseSensor signal.
  if (!pulseSensor.begin()) {
    Serial.println("Pulse sensor initialization failed");
  }
}

void loop() {
      // notify changed value
    if (deviceConnected) {
      Serial.println(pausePolling);
      if(!pausePolling) {
         BPM = pulseSensor.getBeatsPerMinute();
         IBI = pulseSensor.getInterBeatIntervalMs();
         EDA = analogRead(GSR);
      
        pCharacteristic_BPM->setValue((uint8_t*)&BPM, 4);
        pCharacteristic_IBI->setValue((uint8_t*)&IBI, 4);
        pCharacteristic_EDA->setValue((uint8_t*)&EDA, 4);
        pCharacteristic_BPM->notify();
        pCharacteristic_IBI->notify();
        pCharacteristic_EDA->notify();
        
        delay(100); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
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
