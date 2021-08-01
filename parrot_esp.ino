#include "parrot_esp.h"

void setup(){
	Serial.begin(115200);
//	testConvertTools();

	ar_init();

	ble_init();
	for(;;){
		if(ble_scan()) break;
		delay(100);
	}

	ble_connect();
//	ble_enumerateServices();
//	ble_askForSettings();
	ar_sendAllSettings();

	control_init();

}

void loop(){
	if(!ble_checkConnection()){
		ble_connect();
		return;
	}
	control_update();

	ar_checkSendBuffer();
	ar_checkReceiveBuffer();
	ar_checkAckBuffer();
}



void testConvertTools(){
	uint8_t table[] = {0x00, 0x12, 0xCD, 0xFF};

	int16_t value = tools_bufferToInt16t(table);
	uint16_t unsignedValue = tools_bufferToUint16t(table);

	Serial.printf("bytes : ");
	for(uint8_t i = 0; i < 4; ++i){
		Serial.printf("%02X ", table[i]);
	}
	Serial.println();
	Serial.printf("int16_t : %04X\n", value);
	Serial.printf("uint16_t : %04X\n", unsignedValue);

	int32_t value2 = tools_bufferToInt32t(table);
	uint32_t unsignedValue2 = tools_bufferToUint32t(table);

	Serial.printf("int32_t : %08X\n", value2);
	Serial.printf("uint32_t : %08X\n", unsignedValue2);

	Serial.printf("size of float : %i\n", sizeof(float));
	Serial.printf("size of double : %i\n", sizeof(double));

	while(1);
}