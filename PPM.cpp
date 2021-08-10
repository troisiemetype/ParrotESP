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

#include "PPM.h"

hw_timer_t *ppmTimer = NULL;
uint16_t ppmRawChannel[PPM_NUM_CHANNELS];
int16_t ppmChannel[PPM_NUM_CHANNELS];
uint8_t curChan = 0;
volatile bool hasNewData = false;

controlData_t ppmData;

portMUX_TYPE ppmMux = portMUX_INITIALIZER_UNLOCKED;

void ppm_init(){

	memset(&ppmData, 0, sizeof(controlData_t));
	ppmData.channels = ppmChannel;
	ppmData.numChannels = PPM_NUM_CHANNELS;
	ppmData.resolution = 512;

	pinMode(PIN_PPM_INPUT, INPUT);
	attachInterrupt(PIN_PPM_INPUT, _ppm_isr, RISING);

	ppmTimer = timerBegin(0, 80, true);
}

bool ppm_update(){
//	Serial.println(timerRead(ppmTimer));
	if(!hasNewData) return false;
	hasNewData = false;

	for(uint8_t i = 0; i < PPM_NUM_CHANNELS; ++i){
		portENTER_CRITICAL_ISR(&ppmMux);
		ppmChannel[i] = ppmRawChannel[i] - PPM_NEUTRAL;
		portEXIT_CRITICAL_ISR(&ppmMux);
	}
/*
	Serial.printf("channels :\t");
	for(uint8_t i = 0; i < NUM_CHANNELS; ++i){
		Serial.printf("%i\t", channel[i]);
	}
	Serial.println();
*/
	return true;
}

controlData_t* ppm_getChannels(){
	return &ppmData;
}

void IRAM_ATTR _ppm_isr(){
	uint64_t time = timerRead(ppmTimer);
	timerRestart(ppmTimer);
	if(time > 3000){
		curChan = 0;
		hasNewData = true;
	} else {
		portENTER_CRITICAL_ISR(&ppmMux);
		ppmRawChannel[curChan] = (uint16_t)time;
		portEXIT_CRITICAL_ISR(&ppmMux);
		if(++curChan >= PPM_NUM_CHANNELS){
			curChan--;
		}
	}
}