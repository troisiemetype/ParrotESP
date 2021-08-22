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

uint8_t frame[11];

uint8_t frameID = 0;

volatile bool sportUpdate = false;

void sport_init(){
	// baudrate, config, rx pin, tx pin, inverted
	pinMode(PIN_SPORT, OUTPUT);
	sPort.begin(57600, SERIAL_8N1, -1, PIN_SPORT, true);


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
	
	frame[0] = SPORT_START_STOP;
	frame[1] = FRSKY_SPORT_DEVICE_1;
	frame[2] = SPORT_HEADER;		// header answer from device connected to receiver

	switch(frameID){
		case 0:
			tools_uint16tToBuffer(RSSI_ID, &frame[3]);
			tools_uint32tToBuffer(0x64, &frame[5]);
			break;
		case 1:
//			tools_uint16tToBuffer(ADC1_ID, &frame[3]);
//			tools_uint32tToBuffer(0x23, &frame[5]);
			break;
		case 2:
//			tools_uint16tToBuffer(RAS_ID, &frame[3]);
			break;
	}

	for(uint8_t i = 2; i < 9; ++i){
		crc += frame[i];
		crc += crc >> 8;
		crc &= 0xff;
	}

//	frame[9] = 65279 - 257 * crc;			// equivalent to 255 - crc. To be checked.
	frame[9] = 0xff - crc;

	sPort.write(frame, 10);

	if(++frameID > 2) frameID = 0;
/*
	for(uint8_t i = 0; i < 10; ++i){
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
