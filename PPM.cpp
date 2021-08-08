#include "PPM.h"

hw_timer_t *ppmTimer = NULL;
uint16_t ppmRawChannel[NUM_CHANNELS];
int16_t channel[NUM_CHANNELS];
uint8_t curChan = 0;
volatile bool ppmData = false;

portMUX_TYPE ppmMux = portMUX_INITIALIZER_UNLOCKED;

void ppm_init(){

	pinMode(PIN_PPM_INPUT, INPUT);
	attachInterrupt(PIN_PPM_INPUT, ppm_isr, RISING);

	ppmTimer = timerBegin(0, 80, true);
}

bool ppm_update(){
//	Serial.println(timerRead(ppmTimer));
	if(!ppmData) return false;
	ppmData = false;

	for(uint8_t i = 0; i < NUM_CHANNELS; ++i){
		portENTER_CRITICAL_ISR(&ppmMux);
		channel[i] = ppmRawChannel[i] - 1500;
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

int16_t* ppm_getChannels(){
	return channel;
}

void IRAM_ATTR ppm_isr(){
	uint64_t time = timerRead(ppmTimer);
	timerRestart(ppmTimer);
	if(time > 3000){
		curChan = 0;
		ppmData = true;
	} else {
		portENTER_CRITICAL_ISR(&ppmMux);
		ppmRawChannel[curChan] = (uint16_t)time;
		portEXIT_CRITICAL_ISR(&ppmMux);
		if(++curChan >= NUM_CHANNELS){
			curChan--;
		}
	}
}