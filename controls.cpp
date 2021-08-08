#include "controls.h"

controlData_t controlData;

int16_t rawInput[CONTROL_NUM_CHANNELS];
int16_t AETR[4];

bool ch5 = false;
bool prevCh5 = false;

uint8_t ch6 = 0;
uint8_t prevCh6 = 0;

uint8_t ch7 = 0;
uint8_t prevCh7 = 0;

uint8_t ch8 = 0;
uint8_t prevCh8 = 0;

void control_init(){
	memset(rawInput, 0, (CONTROL_NUM_CHANNELS * sizeof(int16_t)));
	memset(&controlData, 0, sizeof(controlData_t));
	controlData.channels = rawInput;
	controlData.numChannels = CONTROL_NUM_CHANNELS;

#if defined TX_USE_PPM
	ppm_init();
	controlData.resolution = ppm_getChannels()->resolution;
#elif defined TX_USE_SBUS
	sbus_init();
	controlData.resolution = sbus_getChannels()->resolution;
#endif
}

void control_update(){
	bool update = false;
	controlData_t *data;

#if defined TX_USE_PPM
	// "rawInput" is pre-foramtted : following the protocol used to decode channels, values are uint16_t, ranging from 0 to 1024.
	// Each protocol function (PPM, S.bus, etc) should then modify them to have unified values 0-centered.

	if(ppm_update()){
		update = true;
		data = ppm_getChannels();
	}

#elif defined TX_USE_SBUS

	if(sbus_update()){
		update = true;
		data = sbus_getChannels();
	}

#endif

	if(update){
		memcpy(rawInput, data->channels, (CONTROL_NUM_CHANNELS * sizeof(int16_t)));
		control_formatControls();
		control_sendAETR();
		control_sendControls();		
	}
/*
	for(uint8_t i = 0; i < CONTROL_NUM_CHANNELS; ++i){
		Serial.printf("%i\t", rawInput[i]);
	}
	Serial.println();
*/
}

void control_formatControls(){
	// minidrones are taking commands mapped from -100 to 100% as input.
	for(uint8_t i = 0; i < 4; ++i){
		AETR[i] = map(rawInput[i], -controlData.resolution, controlData.resolution, -100, 100);
//		Serial.printf("%i\t", AETR[i]);
	}

//	Serial.println();

	prevCh5 = ch5;
	if(rawInput[4] > 0) ch5 = true; else ch5 = false;

	prevCh6 = ch6;
	if(rawInput[5] < -20){
		ch6 = 0;
	} else if(rawInput[5] > 20){
		ch6 = 2;
	} else {
		ch6 = 1;
	}

	prevCh7 = ch7;
	if(rawInput[6] < -20){
		ch7 = 0;
	} else {
		ch7 = 1;
	}

	prevCh8 = ch8;
	if(rawInput[7] < -20){
		ch8 = 0;
	} else {
		ch8 = 1;
	}

//	Serial.printf("ch 6 : %i\n", ch6);
}

// send last read commands from radio to buffers for BLE
// nota : universally used channel order in RC products is AETR : Aileron, Elevator, Throttle, Rudder.
// This is the channel order expected from TX, that you should check.
// However, parrot AR minidrones use another order, which is Roll, Pitch, Yaw, Gaz, i.e. AERT.
// This functions handles the conversion.
void control_sendAETR(){
	ar_sendPCMD(AETR[0], AETR[1], AETR[3], AETR[2]);
//	Serial.printf("AETR\t%i\t%i\t%i\t%i\n", AETR[0], AETR[1], AETR[3], AETR[2]);
}

// Send other channels (non-AETR) to buffers, on change.
void control_sendControls(){
	if(ch5 != prevCh5){
		if(ch5){
			ar_sendFlatTrim();
			ar_sendTakeOff();
//			Serial.println("send take off");
		} else {
			ar_sendLanding();
//			Serial.println("send landing");
		}
	}

	if(ch6 != prevCh6){
//		Serial.println("ch6 triggered");
		switch(ch6){
			case 1:
				ar_sendMaxTilt(20);
				ar_sendMaxVerticalSpeed(1.2);
				ar_sendMaxRotationSpeed(180);
				break;
			case 2:
				ar_sendMaxTilt(25);
				ar_sendMaxVerticalSpeed(2.0);
				ar_sendMaxRotationSpeed(360);
				break;
			case 0:
			default:
				ar_sendMaxTilt(10);
				ar_sendMaxVerticalSpeed(0.6);
				ar_sendMaxRotationSpeed(120);
				break;
		}
	}

	if(ch7 != prevCh7){
		ar_sendTogglePilotingMode();
	}
	
	if(ch8 != prevCh8){
		if(ch8){
			ar_sendBankedTurn(true);
		} else {
			ar_sendBankedTurn(false);
		}
	}
}
