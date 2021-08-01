#include "tools.h"

void tools_int16tToBuffer(int16_t value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 2;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

void tools_uint16tToBuffer(uint16_t value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 2;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

void tools_int32tToBuffer(int32_t value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 4;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

void tools_uint32tToBuffer(uint32_t value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 4;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

void tools_int64tToBuffer(int64_t value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 8;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

void tools_uint64tToBuffer(uint64_t value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 8;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

void tools_floatToBuffer(float value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 4;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

void tools_doubleToBuffer(double value, uint8_t* buffer, bool incBuffer){
	const uint8_t length = 8;
	memcpy(buffer, &value, length);
	if(incBuffer) buffer += length;
}

int16_t tools_bufferToInt16t(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 2;
	int16_t value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}

uint16_t tools_bufferToUint16t(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 2;
	uint16_t value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}

int32_t tools_bufferToInt32t(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 4;
	int32_t value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}

uint32_t tools_bufferToUint32t(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 4;
	uint32_t value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}

int64_t tools_bufferToInt64t(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 8;
	int64_t value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}

uint64_t tools_bufferToUint64t(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 8;
	uint64_t value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}

float tools_bufferToFloat(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 4;
	float value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}

double tools_bufferToDouble(uint8_t* buffer, bool incBuffer){
	const uint8_t length = 8;
	double value = 0;
	memcpy(&value, buffer, length);
	if(incBuffer) buffer += length;
	return value;
}
