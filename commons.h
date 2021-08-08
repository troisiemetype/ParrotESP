#ifndef PARROT_COMMONS_H
#define PARROT_COMMONS_H

#include <Arduino.h>

enum arFrameType_t{
	FRAME_TYPE_ACK = 1,
	FRAME_TYPE_DATA,
	FRAME_TYPE_LOW_LATENCY,
	FRAME_TYPE_DATA_WITH_ACK,
};

enum arFrameStatus_t{
	FRAME_STATUS_WAIT_SEND = 0,
	FRAME_STATUS_WAIT_ACK,
};

struct arBuffer_t{
	arFrameType_t frameType;
	uint8_t sequenceNumber;
	uint8_t data[18];
	uint8_t length;
	uint32_t timestamp;
	int8_t retry;
	arFrameStatus_t frameStatus;
	arBuffer_t* next;
	arBuffer_t* prev;
};

struct arBufferQueue_t{
	arBuffer_t* start;
	arBuffer_t* end;
	arBuffer_t* run;
	arBuffer_t* write;
	size_t size;
	size_t room;
};

struct controlData_t{
//	uint8_t RSSI;
	bool failsafe;
	bool frameLost;
	uint16_t numChannels;
	int16_t *channels;
	uint16_t resolution;
};

#endif