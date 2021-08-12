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

#ifndef PARROT_ESP_H
#define PARROT_ESP_H

#include <Arduino.h>

#include "settings.h"
#include "commons.h"
#include "tools.h"
#include "SD.h"

#include "BLE.h"
#include "AR.h"

#include "controls.h"
#include "telemetry.h"

/*
 *
 *  Parrot ESP
 *
 *  This software is a partial implementation of the Parrot AR SDK on the esp 32, craft to be run on a board plugged in the JR module bay of an RC transmitter
 *  It only BLE-based protocol, i.e. the minidrones (rolling spider, cargo, Mambo)
 *
 *  The program is articulated into several subprograms (*.cpp / *.h pairs), each responsible of a specific task.
 *  Some of them are subdivided into hardware specific implementations.
 *
 *  Each *.h file contains a description of what its role is.
 */

#endif