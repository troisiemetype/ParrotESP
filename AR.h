#ifndef PARROT_ESP_AR_H
#define PARROT_ESP_AR_H

#include <Arduino.h>

#include "parrot_esp.h"

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

	float maxAltitude; 									// 2/8/0 		minidrone/PilotingSettings/MaxAltitude 		Min : 2m
	float maxTilt; 										// 2/8/1 		minidrone/PilotingSettings/MaxTilt 			Default : 15°
	bool bankedTurn;									// 2/8/2 		minidrone/PilotingSettings/BankedTurn
	float maxThrottle;									// 2/8/3 		minidrone/PilotingSettings/MaxThrottle 		between 0 and 1
	arPilotingMode_t preferredPilotingMode;				// 2/8/4 		minidrone/PilotingSettings/PreferredPilotingMode

	float maxVerticalSpeed; 							// 2/1/0 		minidrone/SpeedSettings/MaxVerticalSpeed 	Default : 0.7m/s
	float maxRotationSpeed; 							// 2/1/1 		minidrone/SpeedSettings/MaxRotationSpeed 	Default : 185°/s
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