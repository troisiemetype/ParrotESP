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

#include "parrot_esp.h"

void initSystems(){

//	testConvertTools();

	// Init the commands buffers for communication flow with Minidrone	
	ar_init();

	// Init the BLE layer, then scan BLE servers around. (note : Minidrones are servers, the controller is a client connecting to it)
	ble_init();
	for(;;){
		if(ble_scan()) break;
		delay(100);
	}

	// Initialize connexion. Connects to found device, then enumerate services and characteristics needed.
	ble_firstConnect();
//	ble_enumerateServices();
//	ble_askForSettings();

	// Once connexion is done, Minidrone is asked to enumerate its current settings, before it can accept any command.
	ar_sendAllSettings();
	// Optionnal
	ar_sendPreferredPilotingMode(2);

	// Control from receiver initialisation
	// TODO : inhibit control until the setup is totally made. Including flushing current state.
	control_init();

}

void mainLoop(){
	// Check that connexion is sitll alive, or try to reconnect. Not really tried yet.
	if(!ble_checkConnection()){
		ble_connect();
		return;
	}

	// check for new controls from trasmitter.
	control_update();

	// Check communication buffers.
	ar_checkSendBuffer();
	ar_checkSendWithAckBuffer();
	ar_checkReceiveBuffer();
}

void setup(){
	// Use for "debug" purpose only.
	// Part of the debug informations use the log library made avaiable by the esp ide. Set debug level when compiling.
	Serial.begin(115200);
//	initSystems();
	// Telemetry initialisation.
	telemetry_init();
}

void loop(){
//	mainLoop();
	telemetry_update();
}


// Developpement only.
void testConvertTools(){
	uint8_t table[] = {0x00, 0x12, 0xCD, 0xFF};

	int16_t value = tools_bufferToInt16t(table);
	uint16_t unsignedValue = tools_bufferToUint16t(table);

	Serial.printf("bytes : ");
	for(uint8_t i = 0; i < 4; ++i){
		Serial.printf("%02X ", table[i]);
	}
	Serial.println();
	Serial.printf("int16_t : %04X\n", value);
	Serial.printf("uint16_t : %04X\n", unsignedValue);

	int32_t value2 = tools_bufferToInt32t(table);
	uint32_t unsignedValue2 = tools_bufferToUint32t(table);

	Serial.printf("int32_t : %08X\n", value2);
	Serial.printf("uint32_t : %08X\n", unsignedValue2);

	Serial.printf("size of float : %i\n", sizeof(float));
	Serial.printf("size of double : %i\n", sizeof(double));

	while(1);
}