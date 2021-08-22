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

RCSportEmu sport(PIN_SPORT, 2, 1);

RCSport::data_t *rssi = NULL;
RCSport::data_t *battery = NULL;

ARCommands *tele_md;
extern ARNetwork mdLink;

uint32_t lastUpdate = 0;
uint32_t updateTime = 2000;
/*
void isrSport(){
	sport.isr();
}
*/
void telemetry_init(ARCommands *ar){
	tele_md = ar;

//	timerAttachInterrupt(sport.getTimer(), isrSport, false);

	rssi = sport.addSensor();
	rssi->id = RCSport::RSSI_ID;
//	rssi->value = tele_md->getRSSI();
//	rssi->value = 100;

	battery = sport.addSensor();
	battery->id = RCSport::BATT_ID;
	battery->value = tele_md->getBattery();

//	Serial.println("telemetry init'd");

//	lastUpdate = millis();
}

bool telemetry_update(){
//	Serial.println("update ?");
	if(!sport.update()) return false;
//	Serial.println("yes !");

	battery->value = tele_md->getBattery();
//	rssi->value = tele_md->getRSSI();
	// Check what offset to add.
	rssi->value = 100 + mdLink.getRSSI();
//	Serial.printf("RSSI from MD : %i\n", tele_md->getRSSI());

	return true;
}

void telemetry_start(){
//	Serial.println("telemetry started");
	sport.start();
}

void telemetry_stop(){
//	Serial.println("telemetry stopped");
	sport.stop();
}
