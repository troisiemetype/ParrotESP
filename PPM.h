#ifndef PARROT_ESP_PPM_H
#define PARRTO_ESP_PPM_H

#include <Arduino.h>
#include "settings.h"

void ppm_init();
void ppm_update();
void ppm_isr();

#endif