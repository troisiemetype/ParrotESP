#ifndef PARROT_ESP_BLE_H
#define PARROT_ESP_BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "parrot_esp.h"

void ble_onReceiveCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);
void ble_onReceiveCommandAck(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);
void ble_onReceiveAckCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);
void ble_onReceiveAckLowLatency(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);

void ble_init();
bool ble_scan();
void ble_connect();

bool ble_checkConnection();

void ble_shakeHands();

void ble_setSendCharacteristics();
void ble_setReceiveCharacteristics();
void ble_setHandshakeCharacteristics();

void ble_sendFrame(arBuffer_t* data, uint8_t length);
//void ble_sendFrame(uint8_t* data, uint8_t length);

void ble_enumerateServices();

void ble_askForSettings();

#endif