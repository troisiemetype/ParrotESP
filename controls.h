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

#if defined TX_USE_PPM
#include "PPM.h"
#elif defined TX_USE_SBUS
#include "SBUS.h"
#endif

#define CONTROL_NUM_CHANNELS	NUM_CHANNELS

void control_init();
void control_update();
void control_formatControls();
void control_sendAETR();
void control_sendControls();

#endif