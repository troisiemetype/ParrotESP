#include "BLE.h"

BLEScan *pBLEScan;
BLEClient *pClient;

BLERemoteCharacteristic *chrSendCommand;
BLERemoteCharacteristic *chrSendCommandAck;
BLERemoteCharacteristic *chrSendLowLatency;
BLERemoteCharacteristic *chrSendAck;

BLERemoteCharacteristic *chrReceiveCommand;
BLERemoteCharacteristic *chrReceiveCommandAck;
BLERemoteCharacteristic *chrReceiveAckCommand;
BLERemoteCharacteristic *chrReceiveAckLowLatency;

// Those seems to be needed in some cases, but rolling spider goes well without
BLERemoteCharacteristic *chrNormalFTPTransferring;
BLERemoteCharacteristic *chrNormalFTPGetting;
BLERemoteCharacteristic *chrNormalFTPHandling;

BLERemoteCharacteristic *chrUpdateFTPTransferring;
BLERemoteCharacteristic *chrUpdateFTPGetting;
BLERemoteCharacteristic *chrUpdateFTPHandling;


BLEAdvertisedDevice *rollingSpider;
uint8_t scanTime = 5;
bool connected = false;
//BLEAddress rollingSpiderAddress;

static BLEUUID ARCOMMAND_SENDING_SERVICE_UUID("9a66fa00-0800-9191-11e4-012d1540cb8e");				// Send service

static BLEUUID SEND_DATA_UUID("9a66fa0a-0800-9191-11e4-012d1540cb8e");			// Send command without ack
static BLEUUID SEND_DATA_WITH_ACK_UUID("9a66fa0b-0800-9191-11e4-012d1540cb8e");		// Send command with ack
static BLEUUID SEND_HIGH_PRIORITY_UUID("9a66fa0c-0800-9191-11e4-012d1540cb8e");		// Send low latency (with ack) (send before other commands)
static BLEUUID SEND_ACK_UUID("9a66fa1e-0800-9191-11e4-012d1540cb8e");				// Send ack for received command

static BLEUUID ARCOMMAND_RECEIVING_SERVICE_UUID("9a66fb00-0800-9191-11e4-012d1540cb8e");				// receive service

static BLEUUID RECEIVE_DATA_UUID("9a66fb0f-0800-9191-11e4-012d1540cb8e");		// Receive command without ack
static BLEUUID RECEIVE_DATA_WITH_ACK_UUID("9a66fb0e-0800-9191-11e4-012d1540cb8e");		// Receive command with ack
static BLEUUID RECEIVE_ACK_DATA_UUID("9a66fb1b-0800-9191-11e4-012d1540cb8e");		// ack for sent command
static BLEUUID RECEIVE_ACK_HIGH_PRIORITY_UUID("9a66fb1c-0800-9191-11e4-012d1540cb8e");	// ack for sent low latency

// Handshake UUID. Not needed with my rolling spider
static BLEUUID NORMAL_BLE_FTP_SERVICE_UUID("9a66fd21-0800-9191-11e4-012d1540cb8e");

static BLEUUID NORMAL_FTP_TRANSFERRING_UUID("9a66fd22-0800-9191-11e4-012d1540cb8e");
static BLEUUID NORMAL_FTP_GETTING_UUID("9a66fd23-0800-9191-11e4-012d1540cb8e");
static BLEUUID NORMAL_FTP_HANDLING_UUID("9a66fd24-0800-9191-11e4-012d1540cb8e");

static BLEUUID UPDATE_BLE_FTP_UUID("9a66fd51-0800-9191-11e4-012d1540cb8e");

static BLEUUID UPDATE_FTP_TRANSFERRING_UUID("9a66fd52-0800-9191-11e4-012d1540cb8e");
static BLEUUID UPDATE_FTP_GETTING_UUID("9a66fd53-0800-9191-11e4-012d1540cb8e");
static BLEUUID UPDATE_FTP_HANDLING_UUID("9a66fd54-0800-9191-11e4-012d1540cb8e");



void ble_onReceiveCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){
/*
	Serial.printf("Received %s", pBLERemoteCharacteristic->getUUID().toString().c_str());
	Serial.print(" of data length ");
	Serial.println(length);
*/	Serial.printf("%i\treceived : ", millis());
	for (uint8_t i = 0; i < length; ++i){
		Serial.printf("%i ", data[i]);
	}
	Serial.println();

	ar_populateReceiveBuffer(FRAME_TYPE_DATA, data, length);
}

void ble_onReceiveCommandAck(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){
/*
	Serial.printf("Received with ack %s", pBLERemoteCharacteristic->getUUID().toString().c_str());
	Serial.print(" of data length ");
	Serial.println(length);
*/	Serial.printf("%i\treceived : ", millis());
	for (uint8_t i = 0; i < length; ++i){
		Serial.printf("%i ", data[i]);
	}
	Serial.println();

	// Send ack
	// Note : calling BLECharacteristic.writeValue() from inside a callback function causes a freeze because of internal semaphores ; use flags !
	ar_populateReceiveBuffer(FRAME_TYPE_DATA_WITH_ACK, data, length);
}

void ble_onReceiveAckCommand(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){
	Serial.printf("%i\treceived Ack for command id : %i\n", millis(), data[2]);
	ar_populateReceiveBuffer(FRAME_TYPE_ACK, data, length);
}

void ble_onReceiveAckLowLatency(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* data, size_t length, bool isNotify){
	Serial.printf("%i\treceived Ack for Low Latency id : %i\n", millis(), data[2]);
	ar_populateReceiveBuffer(FRAME_TYPE_ACK, data, length);
}


class ClientCallbacks : public BLEClientCallbacks{
	void onConnect(BLEClient* pClient){
		Serial.println("connected !");
	}

	void onDisconnect(BLEClient* pClient){
		Serial.println("disconnected !");
		for(uint8_t i = 0; i < 32; ++i){
			Serial.print("#");
		}
		Serial.println();
		connected = false;
	}
};

class AdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks{
	void onResult(BLEAdvertisedDevice advertisedDevice){

//		Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());

		if(strncmp(advertisedDevice.getName().c_str(), "RS_", 3)) return;
		
		
		BLEDevice::getScan()->stop();
		rollingSpider = new BLEAdvertisedDevice(advertisedDevice);
		connected = true;

		// note : we connect to the first rolling spider we find.
		// Later we will try and use menus to display found devices on FrSky controller
	}
};

void ble_init(){
	BLEDevice::init("");

	// default power level is P3
	// See if there is a particular way of setting it, it seems better but not that much.
	// External antena is probably needed.
	BLEDevice::setPower(ESP_PWR_LVL_P9);

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

	return connected;
}

// need to see that again. Maybe it's not needed to re-enumerate all services and characterictics if the connection has laready be made once ?
void ble_connect(){
	pClient = BLEDevice::createClient();
	pClient->setClientCallbacks(new ClientCallbacks());

//	Serial.println("Connection...");

	pClient->connect(rollingSpider);

//	Serial.printf("connected to %s\n\n", pClient->getPeerAddress().toString().c_str());

	ble_setSendCharacteristics();
	ble_setReceiveCharacteristics();
//	ble_setHandshakeCharacteristics();

//	ble_shakeHands();
}

bool ble_checkConnection(){
	return connected;
}


void ble_shakeHands(){
	uint8_t toSend[] = {0, 1, 0, 0};
/*
	chrReceiveCommand->writeValue(toSend, 4);
	chrReceiveCommandAck->writeValue(toSend, 4);
	chrReceiveAckCommand->writeValue(toSend, 4);
	chrReceiveAckLowLatency->writeValue(toSend, 4);
*/	
	chrNormalFTPTransferring->writeValue(toSend, 4);
	chrNormalFTPGetting->writeValue(toSend, 4);
	chrNormalFTPHandling->writeValue(toSend, 4);

	chrUpdateFTPTransferring->writeValue(toSend, 4);
	chrUpdateFTPGetting->writeValue(toSend, 4);
	chrUpdateFTPHandling->writeValue(toSend, 4);
}

void ble_setSendCharacteristics(){
	// Set send service
	BLERemoteService *sendService = pClient->getService(ARCOMMAND_SENDING_SERVICE_UUID);
	if(sendService == NULL){
		log_i("failed to create service %s\n", ARCOMMAND_SENDING_SERVICE_UUID.toString().c_str());
		return;		
	} else {
		log_i("created service %s\n", ARCOMMAND_SENDING_SERVICE_UUID.toString().c_str());		
	}

//	Serial.println();

	// Set non-ack send command
	chrSendCommand = sendService->getCharacteristic(SEND_DATA_UUID);
	if(chrSendCommand == NULL){
		log_i("failed to find characteristic %s\n", SEND_DATA_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", SEND_DATA_UUID.toString().c_str());		
	}

	// Set ack send command
	chrSendCommandAck = sendService->getCharacteristic(SEND_DATA_WITH_ACK_UUID);
	if(chrSendCommandAck == NULL){
		log_i("failed to find characteristic %s\n", SEND_DATA_WITH_ACK_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", SEND_DATA_WITH_ACK_UUID.toString().c_str());		
	}

	// Set low-latency send command
	chrSendLowLatency = sendService->getCharacteristic(SEND_HIGH_PRIORITY_UUID);
	if(chrSendLowLatency == NULL){
		log_i("failed to find characteristic %s\n", SEND_HIGH_PRIORITY_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", SEND_HIGH_PRIORITY_UUID.toString().c_str());		
	}

	// Set ack command
	chrSendAck = sendService->getCharacteristic(SEND_ACK_UUID);
	if(chrSendAck == NULL){
		log_i("failed to find characteristic %s\n", SEND_ACK_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", SEND_ACK_UUID.toString().c_str());		
	}

	Serial.println();


}

void ble_setReceiveCharacteristics(){
		// Set receive service
	BLERemoteService *receiveService = pClient->getService(ARCOMMAND_RECEIVING_SERVICE_UUID);

	if(receiveService == NULL){
		log_i("failed to create service %s\n", ARCOMMAND_RECEIVING_SERVICE_UUID.toString().c_str());
		return;
	} else {
		log_i("created service %s\n", ARCOMMAND_RECEIVING_SERVICE_UUID.toString().c_str());
	}

	Serial.println();

	// Set non-ack receive commands (like battery, etc.)
	chrReceiveCommand = receiveService->getCharacteristic(RECEIVE_DATA_UUID);
	if(chrReceiveCommand == NULL){
		log_i("failed to find characteristic %s\n", RECEIVE_DATA_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", RECEIVE_DATA_UUID.toString().c_str());		
	}

	if(chrReceiveCommand->canNotify()){
		chrReceiveCommand->registerForNotify(ble_onReceiveCommand);
		log_i("callback registered for receive commands\n");
	}

	// Set ack receive commands
	chrReceiveCommandAck = receiveService->getCharacteristic(RECEIVE_DATA_WITH_ACK_UUID);
	if(chrReceiveCommandAck == NULL){
		log_i("failed to find characteristic %s\n", RECEIVE_DATA_WITH_ACK_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", RECEIVE_DATA_WITH_ACK_UUID.toString().c_str());		
	}

	if(chrReceiveCommandAck->canNotify()){
		chrReceiveCommandAck->registerForNotify(ble_onReceiveCommandAck);
		log_i("callback registered for receive commands with ack\n");
	}

	// Set receive ack from commands
	chrReceiveAckCommand = receiveService->getCharacteristic(RECEIVE_ACK_DATA_UUID);
	if(chrReceiveAckCommand == NULL){
		log_i("failed to find characteristic %s\n", RECEIVE_ACK_DATA_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", RECEIVE_ACK_DATA_UUID.toString().c_str());		
	}

	if(chrReceiveAckCommand->canNotify()){
		chrReceiveAckCommand->registerForNotify(ble_onReceiveAckCommand);
		log_i("callback registered for acks from commands\n");
	}

	// Set receive ack from low latency
	chrReceiveAckLowLatency = receiveService->getCharacteristic(RECEIVE_ACK_HIGH_PRIORITY_UUID);
	if(chrReceiveAckLowLatency == NULL){
		log_i("failed to find characteristic %s\n", RECEIVE_ACK_HIGH_PRIORITY_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", RECEIVE_ACK_HIGH_PRIORITY_UUID.toString().c_str());		
	}

	if(chrReceiveAckLowLatency->canNotify()){
		chrReceiveAckLowLatency->registerForNotify(ble_onReceiveAckLowLatency);
		log_i("callback registered for acks from low latency\n");
	}

	Serial.println();
}

void ble_setHandshakeCharacteristics(){
	// Set handshake services
	BLERemoteService *sendService = pClient->getService(NORMAL_BLE_FTP_SERVICE_UUID);
	if(sendService == NULL){
		log_i("failed to create service %s\n", NORMAL_BLE_FTP_SERVICE_UUID.toString().c_str());
		return;		
	} else {
		log_i("created service %s\n", NORMAL_BLE_FTP_SERVICE_UUID.toString().c_str());		
	}

//	Serial.println();

	chrNormalFTPTransferring = sendService->getCharacteristic(NORMAL_FTP_TRANSFERRING_UUID);
	if(chrNormalFTPTransferring == NULL){
		log_i("failed to find characteristic %s\n", NORMAL_FTP_TRANSFERRING_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", NORMAL_FTP_TRANSFERRING_UUID.toString().c_str());		
	}

	chrNormalFTPGetting = sendService->getCharacteristic(NORMAL_FTP_GETTING_UUID);
	if(chrNormalFTPGetting == NULL){
		log_i("failed to find characteristic %s\n", NORMAL_FTP_GETTING_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", NORMAL_FTP_GETTING_UUID.toString().c_str());		
	}

	chrNormalFTPHandling = sendService->getCharacteristic(NORMAL_FTP_HANDLING_UUID);
	if(chrNormalFTPHandling == NULL){
		log_i("failed to find characteristic %s\n", NORMAL_FTP_HANDLING_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", NORMAL_FTP_HANDLING_UUID.toString().c_str());		
	}

	sendService = pClient->getService(UPDATE_BLE_FTP_UUID);
	if(sendService == NULL){
		log_i("failed to create service %s\n", UPDATE_BLE_FTP_UUID.toString().c_str());
		return;		
	} else {
		log_i("created service %s\n", UPDATE_BLE_FTP_UUID.toString().c_str());		
	}

//	Serial.println();

	chrUpdateFTPTransferring = sendService->getCharacteristic(UPDATE_FTP_TRANSFERRING_UUID);
	if(chrUpdateFTPTransferring == NULL){
		log_i("failed to find characteristic %s\n", UPDATE_FTP_TRANSFERRING_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", UPDATE_FTP_TRANSFERRING_UUID.toString().c_str());		
	}

	chrUpdateFTPGetting = sendService->getCharacteristic(UPDATE_FTP_GETTING_UUID);
	if(chrUpdateFTPGetting == NULL){
		log_i("failed to find characteristic %s\n", UPDATE_FTP_GETTING_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", UPDATE_FTP_GETTING_UUID.toString().c_str());		
	}

	chrUpdateFTPHandling = sendService->getCharacteristic(UPDATE_FTP_HANDLING_UUID);
	if(chrUpdateFTPHandling == NULL){
		log_i("failed to find characteristic %s\n", UPDATE_FTP_HANDLING_UUID.toString().c_str());
		return;
	} else {
		log_i("found characteristic %s\n", UPDATE_FTP_HANDLING_UUID.toString().c_str());		
	}


}

void ble_sendFrame(arBuffer_t* data, uint8_t length){
//void ble_sendFrame(uint8_t* data, uint8_t length){
	BLERemoteCharacteristic *remote = NULL;

	switch(data->frameType){
		case FRAME_TYPE_ACK:
			remote = chrSendAck;
			break;
		case FRAME_TYPE_DATA:
			remote = chrSendCommand;
			break;
		case FRAME_TYPE_LOW_LATENCY:
			remote = chrSendLowLatency;
			break;
		case FRAME_TYPE_DATA_WITH_ACK:
			remote = chrSendCommandAck;
			break;
	}

	uint8_t toSend[length + 2];

	toSend[0] = data->frameType;
	toSend[1] = data->sequenceNumber;

	for(uint8_t i = 0; i < length; ++i){
		toSend[i + 2] = data->data[i];
	}

	Serial.printf("%i\tsending ", millis());
	for(uint8_t i = 0; i < length + 2; ++i){
		Serial.printf("%i ", toSend[i]);
	}
	Serial.println();
//	Serial.printf("to remote %s\n", remote->toString().c_str());

	remote->writeValue(toSend, length + 2);
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
	chrSendCommandAck->writeValue(toSend, 6);
}