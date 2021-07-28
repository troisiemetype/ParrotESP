#ifndef PARROT_ESP_AR_H
#define PARROT_ESP_AR_H

#include <Arduino.h>

#include "parrot_esp.h"

struct minidroneState_t{
	uint8_t battery;									// 0/5/1 		common/commonState/batteryStateChanged

	int16_t rssi;										// 0/5/7		common/commonState/wifiSignalChanged
	uint8_t linkSignalQuality;							// 0/5/15		common/commonState/linkSignalQuality

	uint8_t headlightLeft;								// 0/23/0		common/headlightsState/intensityChanged
	uint8_t headlightRight;

	bool flyingState;									// 2/3/1 		minidrone/pilotingState/flyingStateChanged
	// todo : manage alert from minidrone/pilotingState/Alerts

	uint8_t pilotingMode;									// 2/3/6 		minidrone/pilotingState/pilotingModeChanged
	// todo : prefered piloting mode
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

void ar_populateReceiveBuffer(arFrameType_t, uint8_t* data, uint8_t length);
void ar_populateSendBuffer(arFrameType_t, uint8_t* data, uint8_t length);

void ar_checkReceiveBuffer();
void ar_checkSendBuffer();

void ar_unpackFrame(uint8_t* data, size_t length);

void ar_unpackFrameCommon(uint8_t* data, size_t length);

void ar_unpackFrameMinidrone(uint8_t* data, size_t length);

void ar_processCommonCommonState(uint8_t* data, size_t length);
void ar_processCommonHeadlightsState(uint8_t* data, size_t length);

void ar_processMinidronePilotingState(uint8_t* data, size_t length);

void ar_processUnused(uint8_t* data, size_t length);

void ar_sendAskForSettings();

void ar_sendFlatrim();
void ar_sendTakeOff();
void ar_sendPCMD(int8_t roll, int8_t pitch, int8_t yaw, int8_t gaz, bool rollPitchFlag);
void ar_sendLanding();
void ar_sendEmergency();
void ar_sendAutoTakeOffMode(uint8_t autoTakeOffMode);
void ar_sendTogglePilotingMode();

void ar_sendMaxAltitude(float maxAltitude);
void ar_sendMaxTilt(float maxTilt);
// Bank turn ?
// Max throttle / Gaz control
void ar_sendPreferredPilotingMode(uint8_t preferredPilotingMode);

void ar_sendMaxVerticalSpeed(float maxVerticalSpeed);
void ar_sendMaxRotationSpeed(float maxRotationSpeed);
void ar_sendWheels(bool wheels);
void ar_sendMaxHorizontalSpeed(float maxHorizontalSpeed);



#endif