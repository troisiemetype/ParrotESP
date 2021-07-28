#include "AR.h"


CircularBuffer<arBuffer_t, 16> bufferReceiveCommand;
CircularBuffer<arBuffer_t, 16> bufferReceiveCommandAck;
CircularBuffer<arBuffer_t, 16> bufferReceiveAck;

CircularBuffer<arBuffer_t, 16> bufferSendAck;
CircularBuffer<arBuffer_t, 16> bufferSendCommand;
CircularBuffer<arBuffer_t, 16> bufferSendCommandAck;
CircularBuffer<arBuffer_t, 16> bufferSendLowLatency;

uint8_t sequenceNumberAck = 0;
uint8_t sequenceNumberCommand = 0;
uint8_t sequenceNumberLowLatency = 0;
uint8_t sequenceNumberCommandAck = 0;


minidroneState_t minidroneState;

void ar_populateReceiveBuffer(arFrameType_t frameType, uint8_t* data, uint8_t length){
	arBuffer_t temp;

	temp.frameType = frameType;
	temp.sequenceNumber = data[1];
	temp.length = length - 2;

	data += 2;
	length -= 2;

	memcpy(temp.data, data, length);

	switch(frameType){
		case FRAME_TYPE_ACK:
			bufferReceiveAck.write(temp);
			break;
		case FRAME_TYPE_DATA:
			bufferReceiveCommand.write(temp);
			break;
		case FRAME_TYPE_LOW_LATENCY:
		case FRAME_TYPE_DATA_WITH_ACK:
			bufferReceiveCommandAck.write(temp);
			break;
	}
}

void ar_populateSendBuffer(arFrameType_t frameType, uint8_t* data, uint8_t length){
	arBuffer_t temp;

	temp.frameType = frameType;
	temp.length = length;
	memcpy(temp.data, data, length);

	switch(frameType){
		case FRAME_TYPE_ACK:
			temp.sequenceNumber = sequenceNumberAck++;
			bufferSendAck.write(temp);
			break;
		case FRAME_TYPE_DATA:
			temp.sequenceNumber = sequenceNumberCommand++;
			bufferSendCommand.write(temp);
			break;
		case FRAME_TYPE_LOW_LATENCY:
			temp.sequenceNumber = sequenceNumberLowLatency++;
			bufferSendLowLatency.write(temp);
			break;
		case FRAME_TYPE_DATA_WITH_ACK:
			temp.sequenceNumber = sequenceNumberCommandAck++;
			bufferSendCommandAck.write(temp);
			break;
	}
/*
	Serial.printf("populating send buffer with %i bytes : %i %i ", temp.length + 2, temp.frameType, temp.sequenceNumber);
	for(uint8_t i = 0; i < temp.length; ++i){
		Serial.printf("%i ", temp.data[i]);
	}
	Serial.println();
*/
}

void ar_checkReceiveBuffer(){
	arBuffer_t temp;

	if(bufferReceiveAck.available()){

	}

	if(bufferReceiveCommandAck.available()){
		temp = bufferReceiveCommandAck.read();
		ar_populateSendBuffer(FRAME_TYPE_ACK, &temp.sequenceNumber, 1);
		ar_unpackFrame(temp.data, temp.length);
	}

	if(bufferReceiveCommand.available()){
		temp = bufferReceiveCommand.read();
		ar_unpackFrame(temp.data, temp.length);
	}
}

void ar_checkSendBuffer(){
	arBuffer_t temp;
	uint8_t* start;
	bool send = true;

	if(bufferSendLowLatency.available()){
//		start = (uint8_t*)bufferSendLowLatency.tail();
		temp = bufferSendLowLatency.read();
	} else if(bufferSendAck.available()){
//		start = (uint8_t*)bufferSendAck.tail();
		temp = bufferSendAck.read();
	} else if(bufferSendCommand.available()){
//		start = (uint8_t*)bufferSendCommand.tail();
		temp = bufferSendCommand.read();
	} else if(bufferSendCommandAck.available()){
//		start = (uint8_t*)bufferSendCommandAck.tail();
		temp = bufferSendCommandAck.read();
	} else {
		send = false;
	}

	if(send){
/*
		Serial.printf("sending buffer with %i bytes : %i %i ", temp.length + 2, temp.frameType, temp.sequenceNumber);
		for(uint8_t i = 0; i < temp.length; ++i){
			Serial.printf("%i ", temp.data[i]);
		}
		Serial.println();
*/
		ble_sendFrame(&temp, temp.length);
	}
}


void ar_unpackFrame(uint8_t* data, size_t length){
//	Serial.printf("unpack frame %i : ", *data);
	length--;
	switch (*(data++)){
		case PROJECT_COMMON:
			// frame common
//			Serial.printf("common\n");
			ar_unpackFrameCommon(data, length);
			break;
		case PROJECT_MINIDRONE:
			// frame minidrone
//			Serial.printf("Minidrone\n");
			ar_unpackFrameMinidrone(data, length);
			break;
		default:
			// drop frame ?
//			Serial.printf("frame error\n");
			break;
	}
}

void ar_unpackFrameCommon(uint8_t* data, size_t length){
//	Serial.printf("class : %i ", *data);
	length--;
	switch(*(data++)){
		case CMN_COMMON_STATE:
			ar_processCommonCommonState(data, length);
			break;
		case CMN_HEADLIGHTS_STATE:
			ar_processCommonHeadlightsState(data, length);
			break;
		default:
//			Serial.printf("unused class\n");
			ar_processUnused(data, length);
			break;
	}

	Serial.println();
}

void ar_unpackFrameMinidrone(uint8_t* data, size_t length){
//	Serial.printf("class : %i ", *data);
	length--;
	switch(*(data++)){
		case MD_PILOTING_STATE:
			ar_processMinidronePilotingState(data, length);
			break;
		default:
//			Serial.printf("unused class\n");
			ar_processUnused(data, length);
			break;
	}

}

void ar_processCommonCommonState(uint8_t* data, size_t length){
	length--;
	switch(*(data++)){
		case 1:
			minidroneState.battery = *data;
			Serial.printf("battery : %i\n", minidroneState.battery);
			break;
		case 7:
			minidroneState.rssi = (*(++data) << 8) + *data;
			Serial.printf("RSSI : %i\n", minidroneState.rssi);
			break;
		case 16:
			minidroneState.linkSignalQuality = *data;
			Serial.printf("link signal quality : %i\n", minidroneState.linkSignalQuality);
			break;
		default:
			break;
	}
}

void ar_processCommonHeadlightsState(uint8_t* data, size_t length){
	length--;
	switch(*(data++)){
		case 0:
			minidroneState.headlightLeft = *(++data);
			minidroneState.headlightRight = *data;
			break;
		default:
			break;
	}
}

void ar_processMinidronePilotingState(uint8_t* data, size_t length){
	length--;
	switch(*(++data)){
		case 1:
			minidroneState.flyingState = *data;
			Serial.printf("flying state : %i\n", minidroneState.flyingState);
			break;
		case 6:
			minidroneState.pilotingMode = *data;
			Serial.printf("piloting mode : %i\n", minidroneState.pilotingMode);
			break;
		default:
			break;
	}
}

void ar_processUnused(uint8_t* data, size_t length){
//	Serial.printf("data length : %i\n", length);
	Serial.printf("unused data : ");

	for(size_t i = 0; i < length; ++i){
		Serial.printf("%i ", data[i]);
	}

	Serial.println();
}

// 0/2/0
// common/settings/allSettings
void ar_sendAskForSettings(){
	uint8_t toSend[] = {0, 2, 0, 0};
	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, 4);
}

// 2/0/0
// minidrone/piloting/flatrim
void ar_sendFlatrim(){

}

// 2/0/1
// minidrone/piloting/takeOff
void ar_sendTakeOff(){

}

// 2/0/2	(non ack)
// minidrone/piloting/PCMD
void ar_sendPCMD(int8_t roll, int8_t pitch, int8_t yaw, int8_t gaz, bool rollPitchFlag = 1){

}

// 2/0/3
// minidrone/piloting/landing
void ar_sendLanding(){

}

// 2/0/4	(low latency)
// minidrone/piloting/emergency
void ar_sendEmergency(){

}

// 2/0/5
// minidrone/piloting/autoTakeOffMode
void ar_sendAutoTakeOffMode(uint8_t autoTakeOffMode){

}

// 2/0/8
// minidrone/piloting/togglePilotingMode
void ar_sendTogglePilotingMode(){

}

// 2/8/0
// minidrone/pilotingSettings/MaxAltitude
void ar_sendMaxAltitude(float maxAltitude){

}

// 2/8/1
// minidrone/pilotingSettings/maxTilt
void ar_sendMaxTilt(float maxTilt){

}

// 2/8/4
// minidrone/pilotingSettings/preferredPilotingMode
void ar_sendPreferredPilotingMode(uint8_t preferredPilotingMode){

}

// 2/1/0
// minidrone/speedSettings/maxVerticalSpeed
void ar_sendMaxVerticalSpeed(float maxVerticalSpeed){

}

// 2/1/1
// minidrone/speedSettings/maxRotationalSpeed
void ar_sendMaxRotationSpeed(float maxRotationSpeed){

}

// 2/1/2
// minidrone/speedSettings/wheels
void ar_sendWheels(bool wheels){

}

// 2/1/3
// minidrone/speedSettings/maxHorizontalSpeed
void ar_sendMaxHorizontalSpeed(float maxHorizontalSpeed){

}

