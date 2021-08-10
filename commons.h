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