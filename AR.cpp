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

#include "AR.h"

/*
 *	Connexion :
 *
 * 	ARSDK protocol mentions the way to communicate with the minidrone.
 * 	However, it seems that the SDK also handles a handshake between the controller and the device, that is not documented.
 * 	The characteristics mentionned in ARSDK starting guide are :
 *		ARcommand sending services		UUID 0xfa00
 *		ARcommand receiving services 	UUID 0xfb00
 *	
 *	Looking at the pyparrot project, it seems that ther are other services needed, which seems to be confirmed by this thread :
 *		https://forum.developer.parrot.com/t/minimal-ble-commands-to-send-for-take-off/1686/2 (which is also mentionned in the code of pyparrot ! :) )
 *	
 *	Those others services are / would be :
 *		Update BLE FTP 					UUID 0xfd51
 *		Normal BLE FTP service 			UUID 0xfd21
 *	(which are indeed enumerated when iterating through all services and characteristics)
 *	
 *	Each of these services enumerate 3 characteristics, each of one has to be sent 0x0100 for the handshake to be completed.
 *
 *	After first tries it seems this handshake is not needed for Rolling Spider !
 */

const size_t SEND_BUFFER_SIZE = 32;
const size_t SEND_WITH_ACK_BUFFER_SIZE = 32;
const size_t RECEIVE_BUFFER_SIZE = 32;


arBuffer_t sendBuffer[SEND_BUFFER_SIZE];
arBufferQueue_t sendQueue;

arBuffer_t sendWithAckBuffer[SEND_BUFFER_SIZE];
arBufferQueue_t sendWithAckQueue;

arBuffer_t receiveBuffer[RECEIVE_BUFFER_SIZE];
arBufferQueue_t receiveQueue;

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
	ar_setBuffer(sendWithAckBuffer, &sendWithAckQueue, SEND_WITH_ACK_BUFFER_SIZE);
	ar_setBuffer(receiveBuffer, &receiveQueue, RECEIVE_BUFFER_SIZE);
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
	Serial.printf("buffer at address %i\n", (int32_t)buffer);
	Serial.printf("\tframe type : %i\n", buffer->frameType);
	Serial.printf("\tsequence number : %i\n", buffer->sequenceNumber);
	Serial.printf("\tdata : ");
	for(uint8_t i = 0; i < buffer->length; ++i){
		Serial.printf("%i ", buffer->data[i]);
	}
	Serial.printf("\n\tlength : %i\n", buffer->length);
	Serial.printf("\ttimestamp : %i\n", buffer->timestamp);
	Serial.printf("\tretry : %i\n", buffer->retry);
	Serial.println();
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

void ar_populateSendBuffer(arFrameType_t frameType, uint8_t* data, uint8_t length, uint8_t retry){
	bool sendWithAck;
	arBufferQueue_t *queue;

	if(frameType == FRAME_TYPE_LOW_LATENCY || frameType == FRAME_TYPE_DATA_WITH_ACK){
		sendWithAck = true;
		queue = &sendWithAckQueue;
	} else {
		sendWithAck = false;
		queue = &sendQueue;
	}

	if(queue->room <= 0) return;

//	Serial.println("populating send buffer");

	arBuffer_t *bf;

	if(frameType == FRAME_TYPE_LOW_LATENCY){
		bf = queue->run->prev;
	} else {
		bf = queue->write;
	}

	bf->frameType = frameType;
	bf->length = length;
	bf->retry = retry;
	bf->frameStatus = FRAME_STATUS_WAIT_SEND;
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
		queue->run = bf;
	} else {
		queue->write = bf->next;
	}
	queue->room--;

//	ar_bufferState(&sendQueue, "queued, send");
//	ar_bufferContent(bf);
}
/*
void ar_populateAckBuffer(arBuffer_t* buffer){
	if(ackQueue.room <= 0) return;

//	Serial.printf("Populating Ack buffer.\n");
	arBuffer_t *bf = ackQueue.write;

	memcpy(bf, buffer, sizeof(arBuffer_t));
	bf->timestamp = millis();

//	ar_bufferContent(bf);
	ackQueue.write = bf->next;
	ackQueue.room--;
}
*/
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
			ar_processAck(bf->data[0]);
			break;
		default:
			break;
	}

	receiveQueue.run = bf->next;
	receiveQueue.room++;
}

void ar_checkSendBuffer(){
	if(sendQueue.room == sendQueue.size) return;

	arBuffer_t *bf = sendQueue.run;

	ble_sendFrame(bf, bf->length);
	sendQueue.run = bf->next;
	sendQueue.room++;

//	ar_bufferState(&sendQueue, "processed, send");
}

void ar_checkSendWithAckBuffer(){
	if(sendWithAckQueue.room == sendWithAckQueue.size) return;

	uint32_t now = millis();

	arBuffer_t *bf = sendWithAckQueue.run;
//	Serial.printf("now : %i\t", now);
//	ar_bufferContent(bf);

	if(bf->frameStatus == FRAME_STATUS_WAIT_SEND){
		bf->timestamp = now;
		ble_sendFrame(bf, bf->length);
		bf->frameStatus = FRAME_STATUS_WAIT_ACK;
	} else if(bf->frameStatus == FRAME_STATUS_WAIT_ACK){
		if((now - bf->timestamp) > FRAME_TIMEOUT){
			if((--bf->retry) > 0){
				bf->timestamp = now;
				ble_sendFrame(bf, bf->length);
				bf->frameStatus = FRAME_STATUS_WAIT_ACK;
			} else {
				sendWithAckQueue.run = bf->next;
				sendWithAckQueue.room++;
			}
		}
	}
}
/*
void ar_checkAckBuffer(){
	if(ackQueue.room == ackQueue.size) return;
//	Serial.println("Checking Ack buffer.");

	uint32_t now = millis();
	arBuffer_t *bf = ackQueue.run;

	if((now - bf->timestamp) > FRAME_TIMEOUT){
//		Serial.printf("timeout %i, re-send buffer %i\n", (now - bf->timestamp), bf->sequenceNumber);
		if(--bf->retry > 0){
			// pass the number of remaining retry
			ar_populateSendBuffer(bf->frameType, bf->data, bf->length, bf->retry);
		}

		ackQueue.run = bf->next;
		ackQueue.room++;
	}
//	ar_bufferState(&ackQueue, "ack");
}
*/
void ar_processAck(uint8_t sequenceNumber){
//	Serial.printf("checking ack id : %i\n", sequenceNumber);
	arBuffer_t *bf = sendWithAckQueue.run;
	if(bf->sequenceNumber == sequenceNumber){
		sendWithAckQueue.run = bf->next;
		sendWithAckQueue.room++;
		memset(bf, 0, sizeof(arBuffer_t));
//		Serial.printf("ack buffer processed : %i\n", (uint32_t)bf);
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


//	ar_bufferState(&receiveQueue, "processed, receive");
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
// 	Serial.printf("class : %i ", *data);
	length--;
	switch(*(data++)){
		case MD_PILOTING_STATE:
			ar_processMinidronePilotingState(data, length);
			break;
		case MD_PILOTING_SETTINGS_STATE:
			ar_processMinidronePilotingSettingsState(data, length);
			break;
		case MD_SPEED_SETTINGS_STATE:
			ar_processMinidroneSpeedSettingsState(data, length);
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
		case 2:
			minidroneState.alertState = (arAlertState_t)(tools_bufferToInt32t(data));
			break;
		case 3:
			minidroneState.autoTakeOffMode = (bool)(*data);
			break;
		case 6:
			minidroneState.pilotingMode = (arPilotingMode_t)tools_bufferToInt32t(data);
			Serial.printf("piloting mode : %i\n", minidroneState.pilotingMode);
			break;
		default:
			break;
	}
}

void ar_processMinidronePilotingSettingsState(uint8_t* data, size_t length){
	length -= 2;
	int16_t command = tools_bufferToUint16t(data, true);
	data += 2;
	switch(command){
		case 0:
			minidroneState.maxAltitude = tools_bufferToFloat(data, true);
			tools_bufferToFloat(data, true);
			tools_bufferToFloat(data, true);
			break;
		case 1:
			minidroneState.maxTilt = tools_bufferToFloat(data, true);
			
			Serial.printf("max tilt : %f (min : %f, max %f)\n", minidroneState.maxTilt,
												tools_bufferToFloat(data, true),
												tools_bufferToFloat(data, true));
			break;
		case 2:
			minidroneState.bankedTurn = *data;
			Serial.printf("banked turn : %i\n", minidroneState.bankedTurn);
			break;
		case 3:
			minidroneState.maxThrottle = tools_bufferToFloat(data, true);
			break;
		case 4:
			minidroneState.preferredPilotingMode = (arPilotingMode_t)tools_bufferToInt32t(data, true);
			break;
		default:
			break;
	}
}

void ar_processMinidroneSpeedSettingsState(uint8_t* data, size_t length){
	length -= 2;
	int16_t command = tools_bufferToUint16t(data, true);
	data += 2;
	switch(command){
		case 0:
		// There are three settings : current vertical speed, min and max.
		// For now only current is saved.
		// the two others could be used to map the user input setting (controller potentiometer) to setting range.
			minidroneState.maxVerticalSpeed = tools_bufferToFloat(data, true);
			
			Serial.printf("max vertical speed : %f (min : %f, max %f)\n", minidroneState.maxVerticalSpeed,
																			tools_bufferToFloat(data, true),
																			tools_bufferToFloat(data, true));

			break;
		case 1:
		// Same here.
			minidroneState.maxRotationSpeed = tools_bufferToFloat(data, true);

			Serial.printf("max rotation speed : %f (min : %f, max %f)\n", minidroneState.maxRotationSpeed,
																			tools_bufferToFloat(data, true),
																			tools_bufferToFloat(data, true));

			break;
		case 2:
		// Wheels. Unused
			break;
		case 3:
			minidroneState.maxHorizontalSpeed = tools_bufferToFloat(data, true);
			tools_bufferToFloat(data, true);
			tools_bufferToFloat(data, true);
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
	if(minidroneState.flyingState != FLYING_STATE_LANDED){
		return;
	}

	const uint8_t length = 4;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING;
	tools_uint16tToBuffer(0, &toSend[2]);


	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);		
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
	if(minidroneState.flyingState != FLYING_STATE_FLYING &&
		minidroneState.flyingState != FLYING_STATE_HOVERING){
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
	if(minidroneState.flyingState == FLYING_STATE_LANDED){
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
	const uint8_t length = 4;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING;
	tools_uint16tToBuffer(4, &toSend[2]);


	ar_populateSendBuffer(FRAME_TYPE_LOW_LATENCY, toSend, length);
}

// 2/0/5
// minidrone/piloting/autoTakeOffMode
void ar_sendAutoTakeOffMode(bool autoTakeOffMode){
	const uint8_t length = 5;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING;
	tools_uint16tToBuffer(5, &toSend[2]);
	toSend[4] = (uint8_t)autoTakeOffMode;


	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/0/8
// minidrone/piloting/togglePilotingMode
void ar_sendTogglePilotingMode(){
	const uint8_t length = 4;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING;
	tools_uint16tToBuffer(8, &toSend[2]);


	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/8/0
// minidrone/pilotingSettings/MaxAltitude
void ar_sendMaxAltitude(float maxAltitude){
	const uint8_t length = 8;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING_SETTINGS;
	tools_uint16tToBuffer(0, &toSend[2]);
	tools_floatToBuffer(maxAltitude, &toSend[4]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/8/1
// minidrone/pilotingSettings/maxTilt
void ar_sendMaxTilt(float maxTilt){
//	Serial.printf("max tilt : %f\n", maxTilt);
	const uint8_t length = 8;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING_SETTINGS;
	tools_uint16tToBuffer(1, &toSend[2]);
	tools_floatToBuffer(maxTilt, &toSend[4]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/8/2
// minidrone/pilotingMode/bankedTurn
void ar_sendBankedTurn(bool bankedTurn){
	const uint8_t length = 5;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING_SETTINGS;
	tools_uint16tToBuffer(2, &toSend[2]);
	toSend[4] = (uint8_t)bankedTurn;

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/8/3
// minidrone/pilotingMode/maxThrottle
void ar_sendMaxThrottle(float maxThrottle){
	const uint8_t length = 8;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING_SETTINGS;
	tools_uint16tToBuffer(3, &toSend[2]);
	tools_floatToBuffer(maxThrottle, &toSend[4]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/8/4
// minidrone/pilotingSettings/preferredPilotingMode
void ar_sendPreferredPilotingMode(uint8_t preferredPilotingMode){
	const uint8_t length = 8;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_PILOTING_SETTINGS;
	tools_uint16tToBuffer(4, &toSend[2]);
	tools_int32tToBuffer((int32_t)preferredPilotingMode, &toSend[4]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);

}

// 2/1/0
// minidrone/speedSettings/maxVerticalSpeed
void ar_sendMaxVerticalSpeed(float maxVerticalSpeed){
	const uint8_t length = 8;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_SPEED_SETTINGS;
	tools_uint16tToBuffer(0, &toSend[2]);
	tools_floatToBuffer(maxVerticalSpeed, &toSend[4]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/1/1
// minidrone/speedSettings/maxRotationalSpeed
void ar_sendMaxRotationSpeed(float maxRotationSpeed){
	const uint8_t length = 8;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_SPEED_SETTINGS;
	tools_uint16tToBuffer(1, &toSend[2]);
	tools_floatToBuffer(maxRotationSpeed, &toSend[4]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/1/2
// minidrone/speedSettings/wheels
void ar_sendWheels(bool wheels){
	const uint8_t length = 5;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_SPEED_SETTINGS;
	tools_uint16tToBuffer(2, &toSend[2]);
	toSend[4] = wheels;

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

// 2/1/3
// minidrone/speedSettings/maxHorizontalSpeed
void ar_sendMaxHorizontalSpeed(float maxHorizontalSpeed){
	const uint8_t length = 8;
	uint8_t toSend[length];
	toSend[0] = PROJECT_MINIDRONE;
	toSend[1] = MD_SPEED_SETTINGS;
	tools_uint16tToBuffer(3, &toSend[2]);
	tools_floatToBuffer(maxHorizontalSpeed, &toSend[4]);

	ar_populateSendBuffer(FRAME_TYPE_DATA_WITH_ACK, toSend, length);
}

int16_t _ar_getCommand(uint8_t* data, size_t* length){
	length -= 2;
	int16_t command = tools_bufferToUint16t(data, true);
	data += 2;

	return command;
}