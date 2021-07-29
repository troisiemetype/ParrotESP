#include "AR.h"

const size_t SEND_BUFFER_SIZE = 32;
const size_t RECEIVE_BUFFER_SIZE = 32;
const size_t ACK_BUFFER_SIZE = 16;

arBuffer_t sendBuffer[SEND_BUFFER_SIZE];
arBufferQueue_t sendQueue;

arBuffer_t receiveBuffer[RECEIVE_BUFFER_SIZE];
arBufferQueue_t receiveQueue;

arBuffer_t ackBuffer[ACK_BUFFER_SIZE];
arBufferQueue_t ackQueue;

uint8_t sequenceNumberAck = 0;
uint8_t sequenceNumberCommand = 0;
uint8_t sequenceNumberLowLatency = 0;
uint8_t sequenceNumberCommandAck = 0;


minidroneState_t minidroneState;

void ar_init(){
	ar_initBuffers();
}

void ar_initBuffers(){
	ar_setBuffer(sendBuffer, &sendQueue, SEND_BUFFER_SIZE);
	ar_setBuffer(receiveBuffer, &receiveQueue, RECEIVE_BUFFER_SIZE);
	ar_setBuffer(ackBuffer, &ackQueue, ACK_BUFFER_SIZE);
//	ar_bufferState(&sendQueue, "send");
//	ar_bufferState(&receiveQueue, "receive");
//	ar_bufferState(&ackQueue, "ack");
}

void ar_setBuffer(arBuffer_t* buffer, arBufferQueue_t* queue, size_t size){
	memset(buffer, 0, sizeof(arBuffer_t) * size);

	uint8_t limit = size - 1;
	for(uint8_t i = 0; i < size; ++i){
		if(i > 0) buffer[i].prev = &buffer[i - 1];
		if(i < limit) buffer[i].next = &buffer[i + 1];
	}

	buffer[0].prev = &buffer[limit];
	buffer[limit].next = buffer;

	queue->start = buffer;
	queue->end = buffer->prev;
	queue->run = buffer;
	queue->write = buffer;
	queue->size = size;
	queue->room = size;
}

void ar_bufferState(arBufferQueue_t* queue, char* name, bool displayBuffer){
	Serial.printf("%s buffer ; used :  %i/%i\n", name, (queue->size - queue->room), queue->size);
	if(displayBuffer) ar_bufferContent(queue->write);
}

void ar_bufferContent(arBuffer_t* buffer){

}


void ar_populateReceiveBuffer(arFrameType_t frameType, uint8_t* data, uint8_t length){
	if(receiveQueue.room <= 0) return;

	arBuffer_t *bf = receiveQueue.write;

	bf->frameType = frameType;
	bf->sequenceNumber = data[1];

	data += 2;
	length -= 2;

	bf->length = length;

	memcpy(bf->data, data, length);

//	ar_bufferState(&receiveQueue, "queued, receive");

	receiveQueue.write = bf->next;
	receiveQueue.room--;

}

void ar_populateSendBuffer(arFrameType_t frameType, uint8_t* data, uint8_t length){
	if(sendQueue.room <= 0) return;

	arBuffer_t *bf;

	if(frameType == FRAME_TYPE_LOW_LATENCY){
		bf = sendQueue.run->prev;
	} else {
		bf = sendQueue.write;
	}

	bf->frameType = frameType;
	bf->length = length;
	memcpy(bf->data, data, length);

	switch(frameType){
		case FRAME_TYPE_ACK:
			bf->sequenceNumber = sequenceNumberAck++;
			break;
		case FRAME_TYPE_DATA:
			bf->sequenceNumber = sequenceNumberCommand++;
			break;
		case FRAME_TYPE_LOW_LATENCY:
			bf->sequenceNumber = sequenceNumberLowLatency++;
			break;
		case FRAME_TYPE_DATA_WITH_ACK:
			bf->sequenceNumber = sequenceNumberCommandAck++;
			break;
	}

	if(frameType == FRAME_TYPE_LOW_LATENCY){
		sendQueue.run = bf;
	} else {
		sendQueue.write = bf->next;
	}
	sendQueue.room--;

//	ar_bufferState(&sendQueue, "queued, send");
}

void ar_checkReceiveBuffer(){
	if(receiveQueue.room == receiveQueue.size) return;
	
	arBuffer_t *bf = receiveQueue.run;

	switch(bf->frameType){
		case FRAME_TYPE_DATA:
			ar_unpackFrame(bf->data, bf->length);
			break;
		case FRAME_TYPE_DATA_WITH_ACK:
			ar_populateSendBuffer(FRAME_TYPE_ACK, &(bf->sequenceNumber), 1);
			ar_unpackFrame(bf->data, bf->length);
			break;
		case FRAME_TYPE_ACK:
			// todo
			receiveQueue.run = receiveQueue.run->next;
			receiveQueue.room++;
//			ar_bufferState(&receiveQueue, "processed, receive");
			break;
		default:
			break;
	}

}

void ar_checkSendBuffer(){
	if(sendQueue.room == sendQueue.size) return;
	ble_sendFrame(sendQueue.run, sendQueue.run->length);
	sendQueue.run = sendQueue.run->next;
	sendQueue.room++;

//	ar_bufferState(&sendQueue, "processed, send");
}


void ar_unpackFrame(uint8_t* data, size_t length){
	Serial.printf("unpack frame %i : ", *data);
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
	receiveQueue.run = receiveQueue.run->next;
	receiveQueue.room++;

//	ar_bufferState(&receiveQueue, "processed, receive");
}

void ar_unpackFrameCommon(uint8_t* data, size_t length){
	Serial.printf("class : %i ", *data);
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
	Serial.printf("class : %i ", *data);
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
	length -= 2;
	int16_t command = tools_bufferToUint16t(data, true);
	data += 2;
	switch(command){
		case 1:
			minidroneState.battery = *data;
			Serial.printf("battery : %i\n", minidroneState.battery);
			break;
		case 7:
			minidroneState.rssi = tools_bufferToInt16t(data);
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
	length -= 2;
	int16_t command = tools_bufferToUint16t(data, true);
	data += 2;
	switch(command){
		case 0:
			minidroneState.headlightLeft = *(++data);
			minidroneState.headlightRight = *data;
			break;
		default:
			break;
	}
}

void ar_processMinidronePilotingState(uint8_t* data, size_t length){
	length -= 2;
	int16_t command = tools_bufferToUint16t(data, true);
	data += 2;
	switch(command){
		case 1:
			minidroneState.flyingState = (arFlyingState_t)tools_bufferToInt32t(data);
			Serial.printf("flying state : %i\n", minidroneState.flyingState);
			break;
		case 6:
			minidroneState.pilotingMode = tools_bufferToInt32t(data);
			Serial.printf("piloting mode : %i\n", minidroneState.pilotingMode);
			break;
		default:
			break;
	}
}

void ar_processUnused(uint8_t* data, size_t length){
//	Serial.printf("data length : %i\n", length);
	uint16_t command = tools_bufferToUint16t(data, true);
	data += 2;
	length -= 2;
	Serial.printf("command : %i, unused data : ", command);

	for(size_t i = 0; i < length; ++i){
		Serial.printf("%i ", data[i]);
	}

	Serial.println();
}

// 0/2/0
// common/settings/allSettings
void ar_sendAllSettings(){
	const uint8_t length = 4;
	uint8_t toSend[length];
	toSend[0] = PROJECT_COMMON;
	toSend[1] = CMN_SETTINGS;
	tools_uint16tToBuffer(0, &toSend[2]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/0/0
// minidrone/piloting/flattrim
void ar_sendFlatTrim(){

}

// 2/0/1
// minidrone/piloting/takeOff
void ar_sendTakeOff(){
	if(minidroneState.flyingState != FLYING_STATE_LANDED){
		return;
	}

	const uint8_t length = 4;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING;
	tools_uint16tToBuffer(1, &toSend[2]);


	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/0/2	(non ack)
// minidrone/piloting/PCMD
void ar_sendPCMD(int8_t roll, int8_t pitch, int8_t yaw, int8_t gaz, bool rollPitchFlag){
	if(minidroneState.flyingState != FLYING_STATE_FLYING){
		return;
	}

	const uint8_t length = 13;
	uint8_t toSend[length];
	memset(toSend, 0, length);
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING;
	tools_uint16tToBuffer(2, &toSend[2]);
	toSend[4] = rollPitchFlag;
	toSend[5] = roll;
	toSend[6] = pitch;
	toSend[7] = yaw;
	toSend[8] = gaz;
	// last four bytes are a timestamp. We will see if really needed.
	ar_populateSendBuffer(FRAME_TYPE_DATA, toSend, length);
}

// 2/0/3
// minidrone/piloting/landing
void ar_sendLanding(){
	if(minidroneState.flyingState != FLYING_STATE_FLYING){
		return;
	}

	const uint8_t length = 4;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING;
	tools_uint16tToBuffer(3, &toSend[2]);


	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
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

