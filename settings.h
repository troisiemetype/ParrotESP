#ifndef PARROT_ESP_SETTINGS_H
#define PARROT_ESP_SETTINGS_H


#define NUM_CHANNELS	8			// max : 8
#define FRAME_TIMEOUT	200			// time before a frame is resent, in milliseconds

// Protocol from TX to esp
// choose only one protocol for communication from radio to esp32 !
#define TX_USE_PPM

// PPM only settings
#define PIN_PPM_INPUT	23

#endif