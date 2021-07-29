/*
#ifndef PARROT_ESP_TX_PROTOCOL
#define PARROT_ESP_TX_PROTOCOL
#else
#error Please choose only one protocol for TX, in settings.h
#endif
*/
#ifndef PARROT_ESP_PPM_H
#define PARRTO_ESP_PPM_H

#include <Arduino.h>

#include "parrot_esp.h"

void ppm_init();
bool ppm_update();

int16_t* ppm_getChannels();
void ppm_isr();

#endif