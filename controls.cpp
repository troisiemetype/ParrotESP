#include "controls.h"

int16_t rawInput[NUM_CHANNELS];
int16_t AETR[4];

bool goFly = false;
bool prevGoFly = false;

uint8_t pilotingMode = 0;
uint8_t prevPilotingMode = 0;

void control_init(){
	memset(rawInput, 0, (NUM_CHANNELS * sizeof(int16_t)));
#ifdef TX_USE_PPM
	ppm_init();
#else
#endif
}

void control_update(){
#ifdef TX_USE_PPM
	if(ppm_update()){
		memcpy(rawInput, ppm_getChannels(), (NUM_CHANNELS * sizeof(int16_t)));
		control_formatControls();
		control_sendAETR();
		control_sendControls();
	}
#else
#endif
}

void control_formatControls(){
	for(uint8_t i = 0; i < 4; ++i){
		AETR[i] = map(rawInput[i], -512, 512, -100, 100);
	}

	prevGoFly = goFly;
	if(rawInput[4] > 0) goFly = true; else goFly = false;

	prevPilotingMode = pilotingMode;
	if(rawInput[5] < 15){
		pilotingMode = 0;
	} else if(rawInput[5] > 15){
		pilotingMode = 2;
	} else {
		pilotingMode = 1;
	}
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
	if(goFly != prevGoFly){
		if(goFly){
			ar_sendTakeOff();
//			Serial.println("send take off");
		} else {
			ar_sendLanding();
//			Serial.println("send landing");
		}
	}

	if(pilotingMode != prevPilotingMode){
		// Nothing for now.
	}

}
