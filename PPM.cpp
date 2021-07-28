#include "PPM.h"

hw_timer_t *ppmTimer = NULL;
uint16_t rawChannel[NUM_CHANNELS];
int16_t channel[NUM_CHANNELS];
uint8_t curChan = 0;
volatile bool ppmData = 0;

portMUX_TYPE ppmMux = portMUX_INITIALIZER_UNLOCKED;

void ppm_init(){

	pinMode(PIN_PPM_INPUT, INPUT);
	attachInterrupt(PIN_PPM_INPUT, ppm_isr, RISING);

	ppmTimer = timerBegin(0, 80, true);
}

void ppm_update(){
//	Serial.println(timerRead(ppmTimer));
	if(!ppmData) return;
	ppmData = 0;

	for(uint8_t i = 0; i < NUM_CHANNELS; ++i){
		portENTER_CRITICAL_ISR(&ppmMux);
		channel[i] = rawChannel[i] - 1500;
		portEXIT_CRITICAL_ISR(&ppmMux);
	}

/*
	Serial.printf("channels :\t");
	for(uint8_t i = 0; i < NUM_CHANNELS; ++i){
		Serial.printf("%i\t", channel[i]);
	}
	Serial.println();
*/
}

void IRAM_ATTR ppm_isr(){
	uint64_t time = timerRead(ppmTimer);
	timerRestart(ppmTimer);
	if(time > 3000){
		curChan = 0;
		ppmData = 1;
	} else {
		portENTER_CRITICAL_ISR(&ppmMux);
		rawChannel[curChan] = (uint16_t)time;
		portEXIT_CRITICAL_ISR(&ppmMux);
		if(++curChan >= NUM_CHANNELS){
			curChan--;
		}
	}
}