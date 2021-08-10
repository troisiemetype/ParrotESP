/*
 * This program is part an esp32-based controller to pilot Parrot Minidrones
 * Copyright (C) 2021  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARROT_ESP_BLE_H
#define PARROT_ESP_BLE_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "parrot_esp.h"

/*
 *  BLE
 *
 *  This part of the program handles bluetooth LE.
 *
 *  Start of bluetooth, scan for Minidrones (servers) around, connection, enumeration of services and characteristics, receive ISR and send function.
 *  UUID of used services and characteristics are set in the BLE.cpp file. Not all sevices and characteristics are used, in fact a lot are not.
 *  Only a few of them are used for controlling the drone, I don't know if the other are documented somewhere !
 *  A list of all services and all their caracteristics can be printed by calling ble_enumerateServices() once the connexion is done.
 *  There is a file in the data folder that lists those services and characteristics on my rolling spider updated with last sofware available.
 */

void ble_onReceiveCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);
void ble_onReceiveCommandAck(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);
void ble_onReceiveAckCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);
void ble_onReceiveAckLowLatency(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify);

void ble_init();
bool ble_scan();
void ble_firstConnect();
void ble_connect();

int ble_getRSSI();
// see if I can : 
// step up esp32 power level.
// step up rollingspider power level as well.

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