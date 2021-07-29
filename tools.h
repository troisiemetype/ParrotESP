#ifndef PARROT_ESP_TOOLS_H
#define PARROT_ESP_TOOLS_H

#include <Arduino.h>

#include "parrot_esp.h"

void tools_int16tToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);
void tools_uint16tToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);

void tools_int32tToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);
void tools_uint32tToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);

void tools_int64tToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);
void tools_uint64tToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);

void tools_floatToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);
void tools_doubleToBuffer(int16_t value, uint8_t* buffer, bool incBuffer = false);


int16_t tools_bufferToInt16t(uint8_t* buffer, bool incBuffer = false);
uint16_t tools_bufferToUint16t(uint8_t* buffer, bool incBuffer = false);

int32_t tools_bufferToInt32t(uint8_t* buffer, bool incBuffer = false);
uint32_t tools_bufferToUint32t(uint8_t* buffer, bool incBuffer = false);

int64_t tools_bufferToInt64t(uint8_t* buffer, bool incBuffer = false);
uint64_t tools_bufferToUint64t(uint8_t* buffer, bool incBuffer = false);

float tools_bufferToFloat(uint8_t* buffer, bool incBuffer = false);
double tools_bufferToDouble(uint8_t* buffer, bool incBuffer = false);

#endif