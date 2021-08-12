const uint8_t PIN_SPORT = 27;
const uint8_t PIN_RX = 17;
const uint8_t PIN_TX = 16;


void setup(){
	Serial.begin(115200);

	// baudrate, config, rx pin, tx pin, inverted
	Serial2.begin(57600, SERIAL_8N1, PIN_SPORT, -1, true);

	Serial.print("init ok.");

}

void loop(){
	while(Serial2.available()){
		Serial.printf("s.port : %02X\n", Serial2.read());
	}

	while	
}