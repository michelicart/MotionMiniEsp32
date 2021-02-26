/*
    Video: https://www.youtube.com/watch?v=oCMOYS71NIU
    Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
    Ported to Arduino ESP32 by Evandro Copercini
    updated by chegewara

   Create a BLE server that, once we receive a connection, will send periodic notifications.
   The service advertises itself as: 4fafc201-1fb5-459e-8fcc-c5c9c331914b
   And has a characteristic of: beb5483e-36e1-4688-b7f5-ea07361b26a8

   The design of creating the BLE server is:
   1. Create a BLE Server
   2. Create a BLE Service
   3. Create a BLE Characteristic on the Service
   4. Create a BLE Descriptor on the characteristic
   5. Start the service.
   6. Start advertising.

   A connect hander associated with the server starts a background task that performs notification
   every couple of seconds.
*/
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;


BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* rpmCharacteristic = NULL;
BLECharacteristic* onoffCharacteristic = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

uint32_t value = 0;
uint32_t valueRPM = 0;
uint32_t valueonoff = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

//Serviço MOtorBB
#define SERVICE_MOTORBB        "716fe929-0a21-4aca-9299-1449b8909e57"
#define CHAR_RPM_TX            "716fe929-0a21-4aca-9299-1449b8909e58"
#define CHAR_ON_OFF_RX         "716fe929-0a21-4aca-9299-1449b8909e59"


class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  pService->start();
  
//  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//  pAdvertising->addServiceUUID(SERVICE_UUID);
//  pAdvertising->setScanResponse(false);
//  pAdvertising->setMinPreferred(0x0);
//  BLEDevice::startAdvertising();
  

/* Serviço MOTOR BE*/
// Create the BLE Service
  BLEService *bbService = pServer->createService(SERVICE_MOTORBB);
//  // Create a BLE Characteristic
  rpmCharacteristic = bbService->createCharacteristic(
                      CHAR_RPM_TX,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

//  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
//  // Create a BLE Descriptor
  rpmCharacteristic->addDescriptor(new BLE2902());
//  
//onoffCharacteristic = bbService->createCharacteristic(
//                      CHAR_ON_OFF_RX,
//                      BLECharacteristic::PROPERTY_READ   |
//                      BLECharacteristic::PROPERTY_WRITE  |
//                      BLECharacteristic::PROPERTY_NOTIFY |
//                      BLECharacteristic::PROPERTY_INDICATE
//                    );
//  // Create a BLE Descriptor
//  onoffCharacteristic->addDescriptor(new BLE2902());
//
//  // Start the service
  pService->start();
//
//  // Start advertising
BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->addServiceUUID(SERVICE_MOTORBB);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
//  BLEAdvertising *bbAdvertising = BLEDevice::getAdvertising();
//  bbAdvertising->addServiceUUID(SERVICE_MOTORBB);
//  bbAdvertising->setScanResponse(false);
//  bbAdvertising->setMinPreferred(0x0);// set value to 0x00 to not advertise this parameter
//  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
    // notify changed value
    if (deviceConnected) {
        pCharacteristic->setValue((uint8_t*)&value, 4);
//        rpmCharacteristic->setValue((uint8_t*)&valueRPM, 4);
//        onoffCharacteristic->setValue((uint8_t*)&valueonoff, 4);
        
        Serial.print(value);
        pCharacteristic->notify();
//        rpmCharacteristic->notify();
//        onoffCharacteristic->notify();
        value++;
        delay(10); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
}
