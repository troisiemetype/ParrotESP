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

#include "controls.h"
#include "espRCSbus.h"

espRCControls::controlData_t *controlData;

espRCSbusIn controlIn(PIN_SBUS_INPUT, 1);

int16_t AETR[4];

bool ch5 = false;
bool prevCh5 = false;

uint8_t ch6 = 0;
uint8_t prevCh6 = 0;

uint8_t ch7 = 0;
uint8_t prevCh7 = 0;

uint8_t ch8 = 0;
uint8_t prevCh8 = 0;
/*
void control_init(){

}
*/

//TODO : inhibit control at beginning to avoid minidrone taking off alone.
void control_update(){
	if(controlIn.update()){
		controlData = controlIn.getChannels();

		_control_formatControls();
		_control_sendAETR();
		_control_sendControls();		
	}
/*
	for(uint8_t i = 0; i < CONTROL_NUM_CHANNELS; ++i){
		Serial.printf("%i\t", controlData->channel[i]);
	}
	Serial.println();
*/
}

void _control_formatControls(){
	// minidrones are taking commands mapped from -100 to 100% as input.
	for(uint8_t i = 0; i < 4; ++i){
		AETR[i] = map(controlData->channel[i], -controlIn.getResolution(), controlIn.getResolution(), -100, 100);
//		Serial.printf("%i\t", AETR[i]);
	}

//	Serial.println();

	prevCh5 = ch5;
	if(controlData->channel[4] > 0) ch5 = true; else ch5 = false;

	prevCh6 = ch6;
	if(controlData->channel[5] < -10){
		ch6 = 0;
	} else if(controlData->channel[5] > 10){
		ch6 = 2;
	} else {
		ch6 = 1;
	}

	prevCh7 = ch7;
	if(controlData->channel[6] < -10){
		ch7 = 0;
	} else {
		ch7 = 1;
	}

	prevCh8 = ch8;
	if(controlData->channel[7] < -10){
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
void _control_sendAETR(){
	ar_sendPCMD(AETR[0], AETR[1], AETR[3], AETR[2]);
//	Serial.printf("AETR\t%i\t%i\t%i\t%i\n", AETR[0], AETR[1], AETR[3], AETR[2]);
}

// Send other channels (non-AETR) to buffers, on change.
void _control_sendControls(){
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
//		Serial.println("ch7 triggered");
		ar_sendTogglePilotingMode();
	}
	
	if(ch8 != prevCh8){
//		Serial.println("ch8 triggered");
		if(ch8){
			ar_sendBankedTurn(true);
		} else {
			ar_sendBankedTurn(false);
		}
	}
}
