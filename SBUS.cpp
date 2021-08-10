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

#include "SBUS.h"

uint8_t sbusInput[25];
uint8_t sbusIndex = 0;
uint8_t sbusLastByte = 0xff;
bool sbusFrameAvailable = false;

uint16_t sbusRawChannel[SBUS_NUM_CHANNELS];
int16_t sbusChannel[SBUS_NUM_CHANNELS];

controlData_t sbusData;

HardwareSerial sBus = HardwareSerial(1);

void sbus_init(){
	// note : Serial (for transmitting to USB-serial chip on dev boards) uses uart 0.
	// thus S.bus should use uart 1 or 2.
//	void HardwareSerial::begin(unsigned long baud, uint32_t config=SERIAL_8N1, int8_t rxPin=-1, int8_t txPin=-1, bool invert=false, unsigned long timeout_ms = 20000UL, uint8_t rxfifo_full_thrhd = 112);
	// todo : only RX pin is implemented ; see if there is a something like a "parking pin" on the esp.
	sBus.begin(100000, SERIAL_8E2, PIN_SBUS_INPUT, -1, true);

	memset(&sbusData, 0, sizeof(sbusData));
	sbusData.channels = sbusChannel;
	sbusData.numChannels = SBUS_NUM_CHANNELS;
	sbusData.resolution = 819;		// todo : define in .h
}

bool sbus_update(){
	while(sBus.available()){
//		Serial.printf("%02X\n", sBus.read());
		uint8_t b = sBus.read();
/*
		Serial.printf("\n%02X\t", b);
		for(uint8_t i = 0; i < 8; ++i){
			Serial.printf("%i", ((b >> (7 - i)) & 0x1));
		}
*/
		if(b == SBUS_HEADER && sbusLastByte == SBUS_FOOTER){
			// new frame
			sbusIndex = 0;
//			Serial.println("header");
		} else if(sbusIndex >= SBUS_FRAME_LENGTH){
			// error, zeroing, so we never exceed array length
			sbusIndex = 0;
//			Serial.println("error");
		} else {
			// incrementing index
			sbusIndex++;
				
		}
		// In any case, we store this byte in the sbus array.
		sbusInput[sbusIndex] = b;
		sbusLastByte = b;
/*
		Serial.printf("%02X\t", b);
		for(uint8_t i = 0; i < 8; ++i){
			Serial.printf("%i", ((b >> (7 - i)) & 0x1));
		}
		Serial.println();
*/
		if(sbusIndex == (SBUS_FRAME_LENGTH - 1) && b == SBUS_FOOTER){
//			Serial.println("footer");
			_sbus_processChannels();
			return true;
		}
	}
	return false;
}

controlData_t* sbus_getChannels(){
	return &sbusData;
}

void _sbus_processChannels(){
	memset(sbusRawChannel, 0, sizeof(int16_t) * SBUS_NUM_CHANNELS);

	// This is kinda odd (at least to me) : but the bytes are transmitted little endian :
	// the upper bits of a channel are to be taken on the little end of the following byte, going from little to big (or from right to left)
	// sbusRawChannel[0] = sbusInput[1] | (((uint16_t)sbusInput[2] << 8) &  0x07FF);

	// Quick and dirty : copy the value one bit at a time from input buffer to channels buffer.
	// I don't want to hardcode each channel as on the example above !
	// Ideally a similar method where copying several bits at a time (as much as possible) would be better.
	for (uint8_t i = 0; i < 176; ++i){
		uint8_t outBitIndex = i % 11;
		uint8_t inBitIndex = i % 8;
		sbusRawChannel[i / 11] |= (((uint16_t)sbusInput[(i / 8) + 1] >> inBitIndex) & 0x1) << outBitIndex;
	}

	sbusChannel[16] = (sbusInput[23] >> 7) & 0x1;
	sbusChannel[17] = (sbusInput[23] >> 6) & 0x1;
	sbusData.frameLost = (sbusInput[23] >> 5) & 0x1;
	sbusData.failsafe = (sbusInput[23] >> 4) & 0x1;

	uint8_t limit = SBUS_NUM_CHANNELS - 2;
	for(uint8_t i = 0; i < SBUS_NUM_CHANNELS; ++i){
		sbusChannel[i] = (int16_t)sbusRawChannel[i] - SBUS_NEUTRAL;
//		Serial.printf("%i\t", sbusChannel[i]);
	}
/*
	Serial.printf("channel 17 : %i\n", sbusChannel[16]);
	Serial.printf("channel 18 : %i\n", sbusChannel[17]);
	Serial.printf("framelost : %i\n", sbusData.frameLost);
	Serial.printf("failsafe : %i\n", sbusData.failsafe);
	Serial.println();
*/
}