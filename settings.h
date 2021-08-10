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

#ifndef PARROT_ESP_SETTINGS_H
#define PARROT_ESP_SETTINGS_H


// #define NUM_CHANNELS	8			// max : 8
#define FRAME_TIMEOUT	200			// time before a frame is resent, in milliseconds

// Protocol from TX to esp
// choose only one protocol for communication from radio to esp32 !
//#define TX_USE_PPM
#define TX_USE_SBUS

#define PIN_RX			16
#define PIN_TX			17
#define PIN_SPORT 		27

// PPM only settings
#define PIN_PPM_INPUT	PIN_RX

// S.bus onlyt settings
#define PIN_SBUS_INPUT	PIN_RX

//#define TELEMETRY_USE_SPORT

#endif