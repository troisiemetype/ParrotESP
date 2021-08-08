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