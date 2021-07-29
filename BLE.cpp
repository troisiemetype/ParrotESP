#include "BLE.h"

BLEScan *pBLEScan;
BLEClient *pClient;

BLERemoteCharacteristic *charSendCommand;
BLERemoteCharacteristic *charSendCommandAck;
BLERemoteCharacteristic *charSendLowLatency;
BLERemoteCharacteristic *charSendAck;

BLERemoteCharacteristic *charReceiveCommand;
BLERemoteCharacteristic *charReceiveCommandAck;
BLERemoteCharacteristic *charReceiveAckCommand;
BLERemoteCharacteristic *charReceiveAckLowLatency;

BLEAdvertisedDevice *rollingSpider;
uint8_t scanTime = 5;
bool connect = false;

static BLEUUID ARSendingUUID("9a66fa00-0800-9191-11e4-012d1540cb8e");				// Send service

static BLEUUID ARSendCommandUUID("9a66fa0a-0800-9191-11e4-012d1540cb8e");			// Send command without ack
static BLEUUID ARSendCommandAckUUID("9a66fa0b-0800-9191-11e4-012d1540cb8e");		// Send command with ack
static BLEUUID ARSendLowLatencyUUID("9a66fa0c-0800-9191-11e4-012d1540cb8e");		// Send low latency (with ack) (send before other commands)
static BLEUUID ARSendAckUUID("9a66fa1e-0800-9191-11e4-012d1540cb8e");				// Send ack for received command

static BLEUUID ARReceivingUUID("9a66fb00-0800-9191-11e4-012d1540cb8e");				// receive service

static BLEUUID ARReceiveCommandUUID("9a66fb0f-0800-9191-11e4-012d1540cb8e");		// Receive command without ack
static BLEUUID ARReceiveCommandAckUUID("9a66fb0e-0800-9191-11e4-012d1540cb8e");		// Receive command with ack
static BLEUUID ARReceiveAckCommandUUID("9a66fb1b-0800-9191-11e4-012d1540cb8e");		// ack for sent command
static BLEUUID ARReceiveAckLowLatencyUUID("9a66fb1c-0800-9191-11e4-012d1540cb8e");	// ack for sent low latency

void ble_onReceiveCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){

	Serial.printf("Received %s", pBLERemoteCharacteristic->getUUID().toString().c_str());
	Serial.print(" of data length ");
	Serial.println(length);
	Serial.print("\tdata: ");
	for (uint8_t i = 0; i < length; ++i){
		Serial.printf("%i ", data[i]);
	}
	Serial.println();

	ar_populateReceiveBuffer(FRAME_TYPE_DATA, data, length);
}

void ble_onReceiveCommandAck(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){

	Serial.printf("Received with ack %s", pBLERemoteCharacteristic->getUUID().toString().c_str());
	Serial.print(" of data length ");
	Serial.println(length);
	Serial.print("\tdata: ");
	for (uint8_t i = 0; i < length; ++i){
		Serial.printf("%i ", data[i]);
	}
	Serial.println();

	// Send ack
	// Note : calling BLECharacteristic.writeValue() from inside a callback function causes a freeze because of internal semaphores ; use flags !
	ar_populateReceiveBuffer(FRAME_TYPE_DATA_WITH_ACK, data, length);
}

void ble_onReceiveAckCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){
	Serial.printf("Received Ack for command id : %i\n", data[2]);
	ar_populateReceiveBuffer(FRAME_TYPE_ACK, data, length);
}

void ble_onReceiveAckLowLatency(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){
	Serial.printf("Received Ack for Low Latency id : %i\n", data[2]);
	ar_populateReceiveBuffer(FRAME_TYPE_ACK, data, length);
}


class ClientCallbacks : public BLEClientCallbacks{
	void onConnect(BLEClient* pClient){
//		Serial.println("connected !");
	}

	void onDisconnect(BLEClient* pClient){
		Serial.println("disconnected !");
		// todo : try to reconnect !
		connect = false;
	}
};

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks{
	void onResult(BLEAdvertisedDevice advertisedDevice){

		Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

		if(strncmp(advertisedDevice.getName().c_str(), "RS_", 3)) return;
		
		
		BLEDevice::getScan()->stop();
		rollingSpider = new BLEAdvertisedDevice(advertisedDevice);
		connect = true;

		// note : we connect to the first rolling spider we find.
		// Later we will try and use menus to display found devices on FrSky controller
	}
};

void ble_init(){
	BLEDevice::init("");

	pBLEScan = BLEDevice::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
	pBLEScan->setActiveScan(true);
	pBLEScan->setInterval(100);
	pBLEScan->setWindow(99);
}

bool ble_scan(){
//	Serial.println("Start scanning");
	BLEScanResults devices = pBLEScan->start(scanTime, false);
//	Serial.printf("%i devices found.\n", devices.getCount());
	pBLEScan->clearResults();

//	Serial.println();

	return connect;
}

void ble_connect(){
	pClient = BLEDevice::createClient();
	pClient->setClientCallbacks(new ClientCallbacks());

//	Serial.println("Connection...");

	pClient->connect(rollingSpider);

	Serial.printf("connected to %s\n\n", pClient->getPeerAddress().toString().c_str());

	// Set send service
	BLERemoteService *sendService = pClient->getService(ARSendingUUID);
	if(sendService == NULL){
		log_i("failed to create service %s\n", ARSendingUUID.toString().c_str());
		return;		
	} else {
		log_i("created service %s\n", ARSendingUUID.toString().c_str());		
	}

//	Serial.println();

	// Set non-ack send command
	charSendCommand = sendService->getCharacteristic(ARSendCommandUUID);
	if(charSendCommand == NULL){
		log_i("failed to find characteristic %s\n", ARSendCommandUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARSendCommandUUID.toString().c_str());		
	}

	// Set ack send command
	charSendCommandAck = sendService->getCharacteristic(ARSendCommandAckUUID);
	if(charSendCommandAck == NULL){
		log_i("failed to find characteristic %s\n", ARSendCommandAckUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARSendCommandAckUUID.toString().c_str());		
	}

	// Set low-latency send command
	charSendLowLatency = sendService->getCharacteristic(ARSendLowLatencyUUID);
	if(charSendLowLatency == NULL){
		log_i("failed to find characteristic %s\n", ARSendLowLatencyUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARSendLowLatencyUUID.toString().c_str());		
	}

	// Set ack command
	charSendAck = sendService->getCharacteristic(ARSendAckUUID);
	if(charSendAck == NULL){
		log_i("failed to find characteristic %s\n", ARSendAckUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARSendAckUUID.toString().c_str());		
	}

	Serial.println();

	// Set receive service
	BLERemoteService *receiveService = pClient->getService(ARReceivingUUID);

	if(receiveService == NULL){
		log_i("failed to create service %s\n", ARReceivingUUID.toString().c_str());
		return;
	} else {
		log_i("created service %s\n", ARReceivingUUID.toString().c_str());
	}

	Serial.println();

	// Set non-ack receive commands (like battery, etc.)
	charReceiveCommand = receiveService->getCharacteristic(ARReceiveCommandUUID);
	if(charReceiveCommand == NULL){
		log_i("failed to find characteristic %s\n", ARReceiveCommandUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARReceiveCommandUUID.toString().c_str());		
	}

	if(charReceiveCommand->canNotify()){
		charReceiveCommand->registerForNotify(ble_onReceiveCommand);
		log_i("callback registered for receive commands\n");
	}

	// Set ack receive commands
	charReceiveCommandAck = receiveService->getCharacteristic(ARReceiveCommandAckUUID);
	if(charReceiveCommandAck == NULL){
		log_i("failed to find characteristic %s\n", ARReceiveCommandAckUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARReceiveCommandAckUUID.toString().c_str());		
	}

	if(charReceiveCommandAck->canNotify()){
		charReceiveCommandAck->registerForNotify(ble_onReceiveCommandAck);
		log_i("callback registered for receive commands with ack\n");
	}

	// Set receive ack from commands
	charReceiveAckCommand = receiveService->getCharacteristic(ARReceiveAckCommandUUID);
	if(charReceiveAckCommand == NULL){
		log_i("failed to find characteristic %s\n", ARReceiveAckCommandUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARReceiveAckCommandUUID.toString().c_str());		
	}

	if(charReceiveAckCommand->canNotify()){
		charReceiveAckCommand->registerForNotify(ble_onReceiveAckCommand);
		log_i("callback registered for acks from commands\n");
	}

	// Set receive ack from low latency
	charReceiveAckLowLatency = receiveService->getCharacteristic(ARReceiveAckLowLatencyUUID);
	if(charReceiveAckLowLatency == NULL){
		log_i("failed to find characteristic %s\n", ARReceiveAckLowLatencyUUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", ARReceiveAckLowLatencyUUID.toString().c_str());		
	}

	if(charReceiveAckLowLatency->canNotify()){
		charReceiveAckLowLatency->registerForNotify(ble_onReceiveAckLowLatency);
		log_i("callback registered for acks from low latency\n");
	}

	Serial.println();
}

void ble_sendFrame(arBuffer_t* data, uint8_t length){
//void ble_sendFrame(uint8_t* data, uint8_t length){
	BLERemoteCharacteristic *remote = NULL;

	switch(data->frameType){
		case FRAME_TYPE_ACK:
			remote = charSendAck;
			break;
		case FRAME_TYPE_DATA:
			remote = charSendCommand;
			break;
		case FRAME_TYPE_LOW_LATENCY:
			remote = charSendLowLatency;
			break;
		case FRAME_TYPE_DATA_WITH_ACK:
			remote = charSendCommandAck;
			break;
	}

	uint8_t toSend[length + 2];

	toSend[0] = data->frameType;
	toSend[1] = data->sequenceNumber;

	for(uint8_t i = 0; i < length; ++i){
		toSend[i + 2] = data->data[i];
	}
/*
	Serial.printf("sending ");
	for(uint8_t i = 0; i < length + 2; ++i){
		Serial.printf("%i ", toSend[i]);
	}
	Serial.printf("to remote %s\n", remote->toString().c_str());
*/
	remote->writeValue(toSend, length);
}

void ble_enumerateServices(){
	// List services
	std::map<std::string, BLERemoteService*>* services = pClient->getServices();
	
	Serial.printf("found %i services :\n", services->size());

	std::map<std::string, BLERemoteService*>::key_compare mycomp = services->key_comp();

	std::string fin = services->rbegin()->first;
	std::map<std::string, BLERemoteService*>::iterator i = services->begin();

	do{
//		Serial.printf("\t%s => %s\n", i->first.c_str(), i->second->toString().c_str());
		Serial.printf("\t%s\n", i->second->toString().c_str());

		// List characteristics per service
		std::map<std::string, BLERemoteCharacteristic*>* chars = i->second->getCharacteristics();		
		std::map<std::string, BLERemoteCharacteristic*>::key_compare mycompchar = chars->key_comp();

		std::string finChar = chars->rbegin()->first;
		std::map<std::string, BLERemoteCharacteristic*>::iterator j = chars->begin();

		do{
//			Serial.printf("\t\t%s => %s\n", j->first.c_str(), j->second->toString().c_str());
			Serial.printf("\t\t%s\n", j->second->toString().c_str());
		} while(mycompchar((*j++).first, finChar));


	} while(mycomp((*i++).first, fin));
	Serial.println("end of services");
}

void ble_askForSettings(){
	uint8_t toSend[] = {4, 0, 0, 2, 0, 0};
	charSendCommandAck->writeValue(toSend, 6);
}