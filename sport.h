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