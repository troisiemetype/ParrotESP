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

#include "sport.h"

hw_timer_t *sPortTimer = NULL;

HardwareSerial sPort = HardwareSerial(2);

uint8_t frame[10];

uint8_t frameID = 0;

volatile bool sportUpdate = false;

void sport_init(){
	// baudrate, config, rx pin, tx pin, inverted
	sPort.begin(57600, SERIAL_8N1, -1, PIN_SPORT, true);
	pinMode(PIN_SPORT, OUTPUT);

	// use timer 1. Timer 0 is used by PPM input.
	sPortTimer = timerBegin(1, 80, true);
	timerAttachInterrupt(sPortTimer, &_sport_isr, false);
	timerAlarmWrite(sPortTimer, 12000, true);
	timerAlarmEnable(sPortTimer);
}

bool sport_update(){
	if(sportUpdate){
		sportUpdate = false;
		_sport_makeFrame();



		return true;
	}

	return false;
}

void IRAM_ATTR _sport_isr(){
	sportUpdate = true;
}

void _sport_makeFrame(){

/*
	_sport_testCRC();
	return;
*/

/* Found on Internet, how a frame is made : 
 * 		0x10 (8bit data frame header)
 *  	value_type (16 bit, e.g. voltage / speed)
 *  	value (32 bit, may be signed or unsigned depending on value type)
 *  	checksum (8 bit)
 *
 *		Total : 8 bytes.
 */
/*
 *	It seems that eveery 12ms (or multiple ?) software ID should be sont, as well as RSSI and battery level.
 *	In multiprotocol, there is start bit added (0x7E).
 *	Then another (0x98) before the frame itself for the three frames exposed above.
 *	For other kind of frames, it's unclear what i should be, 0x1A, 0xBA, or if it should be sent at all. Apparently not sent.
 */

	memset(frame, 0, 10);
	uint16_t crc = 0;
	
	frame[0] = SPORT_HEADER;		// header answer from device connected to receiver

	switch(frameID){
		case 0:
//			tools_uint16tToBuffer(RSSI_ID, &frame[1]);
			frame[1] = 0x01;		// RSSI ID, low byte
			frame[2] = 0xf1;		// RSSI ID, high byte.

			break;
		case 1:
//			tools_uint16tToBuffer(ADC1_ID, &frame[1]);
			frame[1] = 0x02;
			frame[2] = 0xf1;
			break;
		case 2:
//			tools_uint16tToBuffer(RAS_ID, &frame[1]);
			frame[1] = 0x05;
			frame[2] = 0xf1;
			break;
	}

//	frame[1] = 0x01;		// RSSI ID, low byte
//	frame[2] = 0xf1;		// RSSI ID, high byte.
	tools_uint32tToBuffer(0x64, &frame[3]);
//	frame[3] = 0x64;		// Four bytes of data.
//	frame[4] = 0xcd;
//	frame[5] = 0x00;
//	frame[6] = 0x5e;
//	frame[7] = 0x6c;

	// This is the routine used in opentTX to check crc on received S.port packets.

	for(uint8_t i = 1; i < 7; ++i){
		crc += frame[i];
		crc += crc >> 8;
		crc &= 0xff;
	}

//	frame[7] = 65279 - 257 * crc;			// equivalent to 255 - crc. To be checked.
	frame[7] = 0xff - crc;

	sPort.write(SPORT_START_STOP);		// header from receiver to sensors
	switch(frameID){
		case 0:
//			sPort.write(FRSKY_SPORT_DEVICE_24);
//			break;
		case 1:
		case 2:
			sPort.write(FRSKY_SPORT_DEVICE_24);
			break;
	}
	sPort.write(frame, 8);

	if(++frameID > 2) frameID = 0;
/*
	Serial.printf("%02X\n", SPORT_START_STOP);
	Serial.printf("%02X\n", FRSKY_SPORT_DEVICE_24);
	for(uint8_t i = 0; i < 8; ++i){
		Serial.printf("%02X\n", frame[i]);		
	}
	Serial.println();
*/
}


void _sport_testCRC(){
	randomSeed(25);
	uint8_t b = 0;
	uint16_t crc = 0;

	uint8_t table[7];

	Serial.printf("data\tcrc\n");
	for(uint8_t i = 0; i < 6; ++i){
		table[i] = random(256);
		crc += table[i];
		crc += crc >> 8u;
		crc &= 0xffu;
		Serial.printf("%02X\t%02X\n", table[i], crc);
	}

//	table[6] = 65279 - 257 * crc;
	table[6] = 0xFF - crc;

	crc += table[6];
	crc += crc >> 8;
	crc &= 0xff;

	Serial.printf("%02X\t%02X\n\n", table[6], crc);
}
