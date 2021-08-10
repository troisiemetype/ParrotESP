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

#include "telemetry.h"

void telemetry_init(){
#if defined TELEMETRY_USE_SPORT
	sport_init();
#else
	//Nothing
#endif
}

bool telemetry_update(){
	bool update = false;

#if defined TELEMETRY_USE_SPORT
	update = sport_update();
#else
	// nothing
#endif
	
	return update;
}