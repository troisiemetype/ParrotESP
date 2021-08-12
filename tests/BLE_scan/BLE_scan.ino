/*
   Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleScan.cpp
   Ported to Arduino ESP32 by Evandro Copercini
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

int scanTime = 5; //In seconds
BLEScan* pBLEScan;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {

//      if(strncmp(advertisedDevice.getName().c_str(), "RS_", 3)) return;

      Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

//      Serial.printf("address: %s \n", advertisedDevice.getAddress().toString());

      if(advertisedDevice.haveAppearance()){
        Serial.printf("appearance: %i \n", advertisedDevice.getAppearance());
      }

      if(advertisedDevice.haveManufacturerData()){
        Serial.printf("manufacturer data: %s \n", advertisedDevice.getManufacturerData().c_str());
      }

      if(advertisedDevice.haveName()){
        Serial.printf("name: %s \n", advertisedDevice.getName().c_str());
      }

      if(advertisedDevice.haveRSSI()){
        Serial.printf("RSSI: %i \n", advertisedDevice.getRSSI());
      }

      if(advertisedDevice.haveServiceData()){
        Serial.printf("name: %s \n", advertisedDevice.getServiceData().c_str());
      }
/*
      if(advertisedDevice.haveServiceUUID()){
        Serial.printf("name: %s \n", advertisedDevice.getServiceDataUUID());
      }
*/
      if(advertisedDevice.haveTXPower()){
        Serial.printf("name: %i \n", advertisedDevice.getTXPower());
      }


    }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
}

void loop() {
  // put your main code here, to run repeatedly:
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("Scan done!");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  delay(2000);
}