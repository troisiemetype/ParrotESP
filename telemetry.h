#ifndef PARROT_ESP_TELEMETRY_H
#define PARROT_ESP_TELEMETRY_H

#include <Arduino.h>

#include "parrot_esp.h"

#if defined TELEMETRY_USE_SPORT
#include "sport.h"
#endif

void telemetry_init();

bool telemetry_update();

#endif