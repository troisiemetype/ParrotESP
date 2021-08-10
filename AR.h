/*
 * This program is part an esp32-based controller to pilot Parrot Minidrones
 * Copyright (C) 2021  Pierre-Loup Martin
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PARROT_ESP_AR_H
#define PARROT_ESP_AR_H

#include <Arduino.h>

#include "parrot_esp.h"

/*
 *	Parrot-esp - AR
 *
 *	AR is the partial implementation of what Parrot has made available in their sdk, specificly the commands used for communication and control
 *
 *	This part of the program is responsible of creation and management of the communication buffers.
 *	There are enums that reflects the control maps of the SDK, that are used to simplify the writing of new functions
 *	There are also functions that are entry points for sending and reading commands / controls and acknowledgement to and from the drone.
 *
 *	The workflow is organised this way : 
 *		For sending commands / controls, an ar_sendXxx() functions is called from whatever other part of the program.
 *		This function formats the command accordingly, then send it to one of the several buffers used.
 *		Once the buffer has been populated, the ar_checkSendXxxBuffer() will pass this buffer to to the BLE handler, that will send it.
 *
 *		For receiving commands, the BLE handlers will populate the appropriate buffer.
 *		Then the ar_checkReceiveBuffer() function will dispatch it to the parsers, which will eventually set the state structs accordingly.
 *		Or will drop the value into oblivion if it's not implemented or if we don't care.
 *
 *
 *	The control and commands frames are 20 bytes long at most (BLE limitation), and are formatted like this :
 *		Data type
 *		Sequence number
 *		Actual data
 *
 *	Data type can be of for types, each of having it's own BLE characteristic to be sent to:
 *		(1) Ack 				Acknowledgement of previously received data
 *		(2) Data 				Data with no acknowledgement requested (PCMD commands and a few others)
 *		(3) Low latency data 	Data that should have higher priority. (emergency cutoff). This kind of data is pushed at the TAIL of the send buffer.
 *		(4) Data with ack 		Data requesting an ack.
 *
 *		Note on data and acks. A first implementations of buffers in this software sent all commands in buffers, and processed acks as they came.
 *		That didn't work, the minidrones want to have sent their ack for each data before receiving a new one. So the behavior has been changed.
 *
 *	Sequence number is a counter that increments at each new send. Each type of data has it's own. Received sequence number is used when sending ack.
 *
 *	The actual data is formated as follow :
 *		(1 byte)		Project ID 					For minidrones, it's common (0) or minidrone (2)
 *		(1 byte) 		Class ID in the project
 *		(2 bytes)		Command ID in the class
 *		(<=14 bytes)	Data to be sent /received
 *
 *	The data can be signed or unsigned 1, 2, 4 ot 8 bytes int, float or double, null terminated string or enum (treated as 4 bytes).
 *	tools.cpp / .h have functions available to populate and read buffers.
 *
 *	Ach to reveceive data must be sent within 150ms (dixit SDK), but my Rolling Spider rarely answers that fast, so it has been set to 200ms.
 *	The SDK sends PCMD (Piloting CoMmanDs) every 50ms (can't remeber where I read this).
 *	They can be sent more often, but they tend to stack if bellow 20-25ms. Set it on your transmitter if available.
 *
 *	Data that has not been ack whould be resent :
 *		Common data will be sent again up to 4 times.
 *		Low Latency (i.e. high priority, both terms are used by the SDK and other sources, and in this software too) should be sent while not acked to.
 *		PCMD commands are not acked, and a new comand should replace on older that is in buffer and not sent yet. To be implemented.
 *
 *	For more informations about the AR protocol, see this document : https://developer.parrot.com/docs/bebop/ARSDK_Protocols.pdf
 *	You may also look at this github repository, listing all the commands for the AR drones : https://github.com/Parrot-Developers/arsdk-xml/tree/master/xml
 *	You can also have a look at other Parrot's repository on github, there are other informations available.
 *	The parrot developper forum has some valuable informations too : https://forum.developer.parrot.com/c/drone-sdk/6
 *	Lastly, there are other projects aroud nAR drones. This one helped me : https://github.com/amymcgovern/pyparrot
 */

/*
 *	The buffers structure needing to be known by other parts of the program, they are defined in common.h, along with frame types and status.
 */

enum arFlyingState_t{
	FLYING_STATE_LANDED,
	FLYING_STATE_TAKINGOFF,
	FLYING_STATE_HOVERING,
	FLYING_STATE_FLYING,
	FLYING_STATE_LANDING,
	FLYING_STATE_EMERGENCY,
	FLYING_STATE_ROLLING,
	FLYING_STATE_INIT,
};

enum arAlertState_t{
	ALERT_STATE_NONE,
	ALERT_STATE_USER,
	ALERT_STATE_CUTOUT,
	ALERT_STATE_CRITICAL_BATTERY,
	ALERT_STATE_LOW_BATTERY,
};

enum arPilotingMode_t{
	PILOTING_MODE_EASY,
	PILOTING_MODE_MEDIUM,
	PILOTING_MODE_DIFFICULT,
};

struct minidroneState_t{
	uint8_t battery;									// 0/5/1 		common/CommonState/batteryStateChanged

	int16_t rssi;										// 0/5/7		common/CommonState/wifiSignalChanged
	uint8_t linkSignalQuality;							// 0/5/15		common/CommonState/linkSignalQuality

	uint8_t headlightLeft;								// 0/23/0		common/HeadlightsState/intensityChanged
	uint8_t headlightRight;

	arFlyingState_t flyingState;						// 2/3/1 		minidrone/PilotingState/flyingStateChanged
	arAlertState_t alertState;							// 2/3/2 		minidrone/PilotingState/AlertStateChanged
	bool autoTakeOffMode;								// 2/3/3 		minidrone/PilotingState/AutoTakeOffModeChanged
	arPilotingMode_t pilotingMode;						// 2/3/6 		minidrone/PilotingState/pilotingModeChanged		// See also PreferredPilotingMode

	float maxAltitude; 									// 2/8/0 		minidrone/PilotingSettings/MaxAltitude 		min : 2m, max : 10m
	float maxTilt; 										// 2/8/1 		minidrone/PilotingSettings/MaxTilt 			Default : 15° min : 5°, max : 25°
	bool bankedTurn;									// 2/8/2 		minidrone/PilotingSettings/BankedTurn
	float maxThrottle;									// 2/8/3 		minidrone/PilotingSettings/MaxThrottle 		between 0 and 1
	arPilotingMode_t preferredPilotingMode;				// 2/8/4 		minidrone/PilotingSettings/PreferredPilotingMode

	float maxVerticalSpeed; 							// 2/1/0 		minidrone/SpeedSettings/MaxVerticalSpeed 	Default : 0.7m/s, min : 0.5, max : 2.0
	float maxRotationSpeed; 							// 2/1/1 		minidrone/SpeedSettings/MaxRotationSpeed 	Default : 185°/s, min : 50, max : 360
	float maxHorizontalSpeed; 							// 2/1/3 		minidrone/SpeedSettings/MaxHorizontalSpeed  ony used when maxTilt is not used.
	// todo : use Navigation data state ?				// 2/18/x
	// piloting settings state
};

enum{
	PROJECT_COMMON = 0,
	PROJECT_MINIDRONE = 2,
} ar_projects;

enum{
	CMN_NETWORK = 0,
	CMN_NETWORK_EVENT,
	CMN_SETTINGS,
	CMN_SETTINGS_STATE,
	CMN_COMMON,
	CMN_COMMON_STATE,
	CMN_OVERHEAT,
	CMN_OVERHEAT_STATE,
	CMN_CONTROLLER,
	CMN_WIFI_SETTINGS,
	CMN_WIFI_SETTINGS_STATE,
	CMN_MAVLINK,
	CMN_MAVLINK_STATE,
	CMN_CALIBRATION,
	CMN_CALIBRATION_STATE,
	CMN_CAMERA_SETTINGS_STATE,
	CMN_GPS,
	CMN_FLIGHT_PLAN_EVENT,
	CMN_FLIGHT_PLAN_STATE,
	CMN_AR_LIBS_VERSION_STATE,
	CMN_AUDIO,
	CMN_AUDIO_STATE,
	CMN_HEADLIGHTS,
	CMN_HEADLIGHTS_STATE,
	CMN_ANIMATIONS,
	CMN_ANIMATIONS_STATE,
	CMN_ACCESSORY,
	CMN_ACCESSORY_STATE,
	CMN_CHARGER,
	CMN_CHARGER_STATE,
	CMN_RUN_STATE,
	CMN_FACTORY,	
	CMN_FLIGHT_PLAN_SETTINGS,
	CMN_FLIGHT_PLAN_SETTINGS_STATE,
	CMN_UPDATE_STATE,
} ar_commonClasses;

enum{
	MD_PILOTING = 0,
	MD_SPEED_SETTINGS,
	MD_MEDIA_RECORD_EVENT,
	MD_PILOTING_STATE,
	MD_ANIMATIONS,
	MD_SPEED_SETTINGS_STATE,
	MD_MEDIA_RECORD,
	MD_MEDIA_RECORD_STATE,
	MD_PILOTING_SETTINGS,
	MD_PILOTING_SETTINGS_STATE,
	MD_SETTINGS,
	MD_SETTINGS_STATE,
	MD_FLOOD_CONTROL_STATE,
	MD_GPS,
	MD_CONFIGURATION,
	MD_USB_ACCESSORY_STATE,
	MD_USB_ACCESSORY,
	MD_REMOTE_CONTROLLER,
	MD_NAVIGATION_DATA_STATE,
	MD_MINICAM_STATE,
	MD_VIDEO_SETTINGS,
	MD_VIDEO_SETTINGS_STATE,
	MD_MINICAM = 24,
	MD_REMOTE_CONTROLLER_STATE,
} ar_minidroneClasses;

// TODO : make enum for each command in each class in each project ?

void ar_init();
void ar_initBuffers();
void ar_setBuffer(arBuffer_t* buffer, arBufferQueue_t* queue, size_t size);

void ar_bufferState(arBufferQueue_t* queue, char* name, bool displayBuffer = false);
void ar_bufferContent(arBuffer_t* buffer);

void ar_populateReceiveBuffer(arFrameType_t, uint8_t* data, uint8_t length);
void ar_populateSendBuffer(arFrameType_t, uint8_t* data, uint8_t length, uint8_t retry = 5);
void ar_populateAckBuffer(arBuffer_t* buffer);

void ar_checkReceiveBuffer();
void ar_checkSendBuffer();
void ar_checkSendWithAckBuffer();
void ar_checkAckBuffer();

void ar_processAck(uint8_t sequenceNumber);

void ar_unpackFrame(uint8_t* data, size_t length);
void ar_unpackFrameCommon(uint8_t* data, size_t length);
void ar_unpackFrameMinidrone(uint8_t* data, size_t length);

void ar_processCommonCommonState(uint8_t* data, size_t length);
void ar_processCommonHeadlightsState(uint8_t* data, size_t length);

void ar_processMinidronePilotingState(uint8_t* data, size_t length);
void ar_processMinidronePilotingSettingsState(uint8_t* data, size_t length);
void ar_processMinidroneSpeedSettingsState(uint8_t* data, size_t length);

void ar_processUnused(uint8_t* data, size_t length);

void ar_sendAllSettings();

void ar_sendFlatTrim();
void ar_sendTakeOff();
void ar_sendPCMD(int8_t roll, int8_t pitch, int8_t yaw, int8_t gaz, bool rollPitchFlag = true);
void ar_sendLanding();
void ar_sendEmergency();
void ar_sendAutoTakeOffMode(bool autoTakeOffMode);
void ar_sendTogglePilotingMode();

void ar_sendMaxAltitude(float maxAltitude);
void ar_sendMaxTilt(float maxTilt);
void ar_sendBankedTurn(bool bankedTurn);
void ar_sendMaxThrottle(float maxThrottle);
void ar_sendPreferredPilotingMode(uint8_t preferredPilotingMode);

void ar_sendMaxVerticalSpeed(float maxVerticalSpeed);
void ar_sendMaxRotationSpeed(float maxRotationSpeed);
void ar_sendWheels(bool wheels);
void ar_sendMaxHorizontalSpeed(float maxHorizontalSpeed);

int16_t _ar_getCommand(uint8_t* data, size_t* length);

#endif