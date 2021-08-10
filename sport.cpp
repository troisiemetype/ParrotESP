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

uint8_t frame[8];

volatile bool sportUpdate = false;

void sport_init(){
	// baudrate, config, rx pin, tx pin, inverted
	sPort.begin(57600, SERIAL_8N1, PIN_SPORT, PIN_SPORT, true);

	// use timer 1. Timer 0 is used by PPM input.
	sPortTimer = timerBegin(1, 80, true);
	timerAttachInterrupt(sPortTimer, &_sport_isr, true);
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

void _sport_isr(){
	sportUpdate = true;
}

void _sport_makeFrame(){

/* Found on Internet, how a frame is made : 
 * 		0x10 (8bit data frame header)
 *  	value_type (16 bit, e.g. voltage / speed)
 *  	value (32 bit, may be signed or unsigned depending on value type)
 *  	checksum (8 bit)
 */

	uint16_t crc = 0;
	
	frame[0] = 0x10;
	tools_uint16tToBuffer(BAT_ID, &frame[1]);
	tools_uint32tToBuffer(80, &frame[3]);

	// This is the routine used in opentTX to check crc on received S.port packets.

	for(uint8_t i = 1; i < 7; ++i){
		crc += frame[i];
		crc += crc >> 8;
		crc &= 0xff;
	}

	frame[7] = 65279 - 257 * crc;

	sPort.write(frame, 8);

}

/*
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

	table[6] = 65279 - 257 * crc;

	crc += table[6];
	crc += crc >> 8;
	crc &= 0xff;

	Serial.printf("%02X\t%02X\n\n", table[6], crc);
}
*/