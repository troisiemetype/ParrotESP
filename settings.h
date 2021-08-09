#ifndef PARROT_ESP_SETTINGS_H
#define PARROT_ESP_SETTINGS_H


// #define NUM_CHANNELS	8			// max : 8
#define FRAME_TIMEOUT	200			// time before a frame is resent, in milliseconds

// Protocol from TX to esp
// choose only one protocol for communication from radio to esp32 !
//#define TX_USE_PPM
#define TX_USE_SBUS

#define PIN_RX			16
#define PIN_TX			17
#define PIN_SPORT 		27

// PPM only settings
#define PIN_PPM_INPUT	PIN_RX

// S.bus onlyt settings
#define PIN_SBUS_INPUT	PIN_RX

//#define TELEMETRY_USE_SPORT

#endif