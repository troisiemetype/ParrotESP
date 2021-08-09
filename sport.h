#ifndef PARROT_ESP_SPORT_H
#define PARROT_ESP_SPORT_H

/*
 *	S.port
 *	S.port is a telemetry protocol from FrSky.
 *	It's a half duplex serial link where a master polls slaves, and slaves responds when called.
 *	
 */

#include <Arduino.h>

#include "parrot_esp.h"

#define BAT_ID				0xF104 			// opentx/frsky.h

void sport_init();
bool sport_update();

void _sport_isr();

void _sport_makeFrame();

void _sport_testCRC();

#endif