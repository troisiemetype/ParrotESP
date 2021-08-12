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

#ifndef PARROT_ESP_CONTROLS_H
#define PARROT_ESP_CONTROLS_H

#include <Arduino.h>

#include "parrot_esp.h"

/*
 *  Controls
 *
 *  This is the entry point to read controls from the transmitter the module is plugged to.
 *  There are mainly two functions, control_init() and control_update(), that are called during init and program run, respectively.
 *  Those functions bridge between the hardware-dedicated functions, like PPM or S.bus, and the AR_ functions that sends commands to the drone.
 *  Other functions chould only be used internally (_prepended)
 *
 *  Hardware drivers should implement three functions that are called by control_xxx :
 *      void xxx_init()
 *      bool xxx_update()                       that should return 1 if new data has been received. It returns 1 only once.
 *      controlData_t * xxx_getChannels()       that returns a pointer to a struct holding the data for each channels, as well as some other informations.
 *                                              Note : this struct may evolve again.
 */

// Only one input protocol (from transmitter) should be #defined in the settings.h file.
// TODO : implement a preprocessor guard for multiple definitions.

#if defined (TX_USE_PPM)
#include "PPM.h"
#elif defined (TX_USE_SBUS)
#include "SBUS.h"
#else
#error "no protocol defined for control input."
#endif

#define CONTROL_NUM_CHANNELS	NUM_CHANNELS

void control_init();
void control_update();
void _control_formatControls();
void _control_sendAETR();
void _control_sendControls();

#endif