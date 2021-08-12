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

#ifndef PARROT_ESP_TELEMETRY_H
#define PARROT_ESP_TELEMETRY_H

#include <Arduino.h>

#include "parrot_esp.h"

#if defined TELEMETRY_USE_SPORT
#include "sport.h"
#else
#error "no protocol defined for telemetry."
#endif

void telemetry_init();

bool telemetry_update();

#endif