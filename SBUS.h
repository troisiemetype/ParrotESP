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

#ifndef PARROT_ESP_SBUS_H
#define PARROT_ESP_SBUS_H

/*
 *	S.bus protocol
 *
 *	S.bus is a digital protocol for transmitting up to 18 channels (16 variable + 2 boolean) on a single line.
 *	It is a monodirectionnal protocol
 *	It is used both to send data on a transmitter, from CPU to transmitting module, and on receiver, to send received data to servos.
 *	S.bus is a inverted (to be checked) UART, using only TX (or RX when receiving), with a baudarate of 100000, 8 data bits, even parity, 2 stops bits.
 *
 *	Each frame is composed of 25 bytes, following this pattern :
 *
 *	[0] 	header byte, 0x0F
 *	[1-22]	channels 1-16, 11 bits per channel
 *	[23]	four flags : 
 *				bit 7	channel 17 (boolean)
 *				bit 6 	channel 18 (boolean)
 *				bit 5 	frame lost
 *				bit 4 	fail safe activated
 *	[24]	fotter byte, 0x00
 *
 *	Notes :
 *		frame are transmitted every 10 to 20ms.
 *		channels 17 & 18 are not universally used.
 *		lost frame flag is used on the receiver side to indicate that a frame has been lost.
 *		fail safef is also used by the receiver to indicate that this mode has been entered, following several lost frames.
 *		fulle range for servos (0-2047, 11 bits) correspond to extended limits range, -150% - 150% (to be sonfirmed by testing)
 */
#include <Arduino.h>
#include "parrot_esp.h"

#define SBUS_FRAME_LENGTH       25
#define SBUS_HEADER             0x0F
#define SBUS_FOOTER             0x00
//#define SBUS_FOOTER_ALT           0x7C        // need to find why the footer can change on may radio (and also why this footer has 2 bytes less per frame)

#define SBUS_NUM_CHANNELS       18
#define NUM_CHANNELS            SBUS_NUM_CHANNELS

#define SBUS_NEUTRAL            992             // Why is it not 1024 ? On openTX it's 992.

void sbus_init();
bool sbus_update();

controlData_t* sbus_getChannels();

void _sbus_processChannels();

#endif