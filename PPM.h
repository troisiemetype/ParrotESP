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

/*
#ifndef PARROT_ESP_TX_PROTOCOL
#define PARROT_ESP_TX_PROTOCOL
#else
#error Please choose only one protocol for TX, in settings.h
#endif
*/

/*
 * PPM driver.
 * PPM is a quite simple protocol, inherited from the first RC systems, and still widely used.
 * The principle is quite simple :
 * 			channels are transmitted in a row on a single line, using impulsions.
 *			channel value is the duration between to consequent pulses.
 *			a neutral is a duration of length 1,5ms, min is around 0,9ms, max is around 2,1ms.
 *			when all channels have been transmitted, the line stays low until the next frame.
 *			this padding is used to detect a frame end, and subsequently the start of the next one
 *
 * ch1------ch2------ch3--ch4----------ch5-----ch6------ch7--ch8----padding----------------
 * |________|________|____|____________|_______|________|____|______|______________________
 *
 * Old analogs radios were transmitting frames at a 20Hz frequency (one frame = 50ms), and where transmitting a max of 8 channels.
 * On newer radios the frequency can be adjusted, and up to 16 channels transmitted.
 *
 * Neutral used to vary from one manufacturer to another, from 1,2 to 1,6ms.
 *
 * Note : since analog servos still use this protocol to control their position, neutral length may need to be adjusted.
 */

#ifndef PARROT_ESP_PPM_H
#define PARRTO_ESP_PPM_H

#include <Arduino.h>

#include "parrot_esp.h"

#define PPM_NEUTRAL         1500
#define PPM_NUM_CHANNELS    8
#define NUM_CHANNELS        PPM_NUM_CHANNELS

void ppm_init();
bool ppm_update();

controlData_t* ppm_getChannels();
void _ppm_isr();

#endif