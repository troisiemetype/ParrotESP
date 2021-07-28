#include "parrot_esp.h"
#include "settings.h"
#include "PPM.h"
#include "BLE.h"
#include "AR.h"

void setup(){
	Serial.begin(115200);
//	ppm_init();
	ble_init();
	for(;;){
		if(ble_scan()) break;
		delay(1000);
	}

	ble_connect();
//	ble_enumerateServices();
	ar_sendAskForSettings();
}

void loop(){
//	ppm_update();
	ar_checkSendBuffer();
	ar_checkReceiveBuffer();
}