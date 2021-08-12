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

#ifndef PARROT_ESP_SPORT_H
#define PARROT_ESP_SPORT_H

/*
 *	S.port
 *	S.port is a telemetry protocol from FrSky.
 *	It's a half duplex serial link where a master polls slaves, and slaves responds when called.
 *
 *  A huge thanks to RomanLut@github for his help. I had telemetry working for the first time thanks to his advices.
 */

#include <Arduino.h>

#include "parrot_esp.h"

#define SPORT_START_STOP		0x7e
#define SPORT_HEADER		  	0x10
#define SPORT_BYTESTUFF			0x7d
#define SPORT_SUTFF_MASK		0x20

// From multiwii :
// frsky sensor IDs
// Comments for devices 0 to 6 from openTX, frsky.h
												// Vario @ ID 0, 0x00
#define FRSKY_SPORT_DEVICE_1 	0xa1			// FLVSS
#define FRSKY_SPORT_DEVICE_2 	0x22			// FAS
#define FRSKY_SPORT_DEVICE_3 	0x83			// GPS
#define FRSKY_SPORT_DEVICE_4 	0xe4			// RPM
#define FRSKY_SPORT_DEVICE_5 	0x45			// SP2UH
#define FRSKY_SPORT_DEVICE_6 	0xc6			// SP2UR
#define FRSKY_SPORT_DEVICE_7 	0x67
#define FRSKY_SPORT_DEVICE_8 	0x48
#define FRSKY_SPORT_DEVICE_9 	0xe9
#define FRSKY_SPORT_DEVICE_10	0x6a
#define FRSKY_SPORT_DEVICE_11	0xcb
#define FRSKY_SPORT_DEVICE_12	0xac
#define FRSKY_SPORT_DEVICE_13	0x0d
#define FRSKY_SPORT_DEVICE_14	0x8e
#define FRSKY_SPORT_DEVICE_15	0x2f
#define FRSKY_SPORT_DEVICE_16	0xd0
#define FRSKY_SPORT_DEVICE_17	0x71
#define FRSKY_SPORT_DEVICE_18	0xf2
#define FRSKY_SPORT_DEVICE_19	0x53
#define FRSKY_SPORT_DEVICE_20	0x34
#define FRSKY_SPORT_DEVICE_21	0x95
#define FRSKY_SPORT_DEVICE_22	0x16
#define FRSKY_SPORT_DEVICE_23	0xb7
#define FRSKY_SPORT_DEVICE_24	0x98
#define FRSKY_SPORT_DEVICE_25	0x39
#define FRSKY_SPORT_DEVICE_26	0xba
#define FRSKY_SPORT_DEVICE_27	0x1b

// from openTX, frsky.h
// FrSky new DATA IDs (2 bytes)
#define ALT_FIRST_ID              0x0100D        
#define ALT_LAST_ID               0x010F
#define VARIO_FIRST_ID            0x0110
#define VARIO_LAST_ID             0x011F
#define CURR_FIRST_ID             0x0200
#define CURR_LAST_ID              0x020F
#define VFAS_FIRST_ID             0x0210
#define VFAS_LAST_ID              0x021F
#define CELLS_FIRST_ID            0x0300
#define CELLS_LAST_ID             0x030F
#define T1_FIRST_ID               0x0400
#define T1_LAST_ID                0x040F
#define T2_FIRST_ID               0x0410
#define T2_LAST_ID                0x041F
#define RPM_FIRST_ID              0x0500
#define RPM_LAST_ID               0x050F
#define FUEL_FIRST_ID             0x0600
#define FUEL_LAST_ID              0x060F
#define ACCX_FIRST_ID             0x0700
#define ACCX_LAST_ID              0x070F
#define ACCY_FIRST_ID             0x0710
#define ACCY_LAST_ID              0x071F
#define ACCZ_FIRST_ID             0x0720
#define ACCZ_LAST_ID              0x072F
#define GPS_LONG_LATI_FIRST_ID    0x0800
#define GPS_LONG_LATI_LAST_ID     0x080F
#define GPS_ALT_FIRST_ID          0x0820
#define GPS_ALT_LAST_ID           0x082F
#define GPS_SPEED_FIRST_ID        0x0830
#define GPS_SPEED_LAST_ID         0x083F
#define GPS_COURS_FIRST_ID        0x0840
#define GPS_COURS_LAST_ID         0x084F
#define GPS_TIME_DATE_FIRST_ID    0x0850
#define GPS_TIME_DATE_LAST_ID     0x085F
#define A3_FIRST_ID               0x0900
#define A3_LAST_ID                0x090F
#define A4_FIRST_ID               0x0910
#define A4_LAST_ID                0x091F
#define AIR_SPEED_FIRST_ID        0x0A00
#define AIR_SPEED_LAST_ID         0x0A0F
#define FUEL_QTY_FIRST_ID         0x0A10
#define FUEL_QTY_LAST_ID          0x0A1F
#define RBOX_BATT1_FIRST_ID       0x0B00
#define RBOX_BATT1_LAST_ID        0x0B0F
#define RBOX_BATT2_FIRST_ID       0x0B10
#define RBOX_BATT2_LAST_ID        0x0B1F
#define RBOX_STATE_FIRST_ID       0x0B20
#define RBOX_STATE_LAST_ID        0x0B2F
#define RBOX_CNSP_FIRST_ID        0x0B30
#define RBOX_CNSP_LAST_ID         0x0B3F
#define SD1_FIRST_ID              0x0B40
#define SD1_LAST_ID               0x0B4F
#define ESC_POWER_FIRST_ID        0x0B50
#define ESC_POWER_LAST_ID         0x0B5F
#define ESC_RPM_CONS_FIRST_ID     0x0B60
#define ESC_RPM_CONS_LAST_ID      0x0B6F
#define ESC_TEMPERATURE_FIRST_ID  0x0B70
#define ESC_TEMPERATURE_LAST_ID   0x0B7F
#define RB3040_OUTPUT_FIRST_ID    0x0B80
#define RB3040_OUTPUT_LAST_ID     0x0B8F
#define RB3040_CH1_2_FIRST_ID     0x0B90
#define RB3040_CH1_2_LAST_ID      0x0B9F
#define RB3040_CH3_4_FIRST_ID     0x0BA0
#define RB3040_CH3_4_LAST_ID      0x0BAF
#define RB3040_CH5_6_FIRST_ID     0x0BB0
#define RB3040_CH5_6_LAST_ID      0x0BBF
#define RB3040_CH7_8_FIRST_ID     0x0BC0
#define RB3040_CH7_8_LAST_ID      0x0BCF
#define X8R_FIRST_ID              0x0C20
#define X8R_LAST_ID               0x0C2F
#define S6R_FIRST_ID              0x0C30
#define S6R_LAST_ID               0x0C3F
#define GASSUIT_TEMP1_FIRST_ID    0x0D00
#define GASSUIT_TEMP1_LAST_ID     0x0D0F
#define GASSUIT_TEMP2_FIRST_ID    0x0D10
#define GASSUIT_TEMP2_LAST_ID     0x0D1F
#define GASSUIT_SPEED_FIRST_ID    0x0D20
#define GASSUIT_SPEED_LAST_ID     0x0D2F
#define GASSUIT_RES_VOL_FIRST_ID  0x0D30
#define GASSUIT_RES_VOL_LAST_ID   0x0D3F
#define GASSUIT_RES_PERC_FIRST_ID 0x0D40
#define GASSUIT_RES_PERC_LAST_ID  0x0D4F
#define GASSUIT_FLOW_FIRST_ID     0x0D50
#define GASSUIT_FLOW_LAST_ID      0x0D5F
#define GASSUIT_MAX_FLOW_FIRST_ID 0x0D60
#define GASSUIT_MAX_FLOW_LAST_ID  0x0D6F
#define GASSUIT_AVG_FLOW_FIRST_ID 0x0D70
#define GASSUIT_AVG_FLOW_LAST_ID  0x0D7F
#define SBEC_POWER_FIRST_ID       0x0E50
#define SBEC_POWER_LAST_ID        0x0E5F
#define DIY_FIRST_ID              0x5100
#define DIY_LAST_ID               0x52FF
#define DIY_STREAM_FIRST_ID       0x5000
#define DIY_STREAM_LAST_ID        0x50FF
#define SERVO_FIRST_ID            0x6800
#define SERVO_LAST_ID             0x680F
#define FACT_TEST_ID              0xF000
#define VALID_FRAME_RATE_ID       0xF010
#define RSSI_ID                   0xF101
#define ADC1_ID                   0xF102
#define ADC2_ID                   0xF103
#define BATT_ID                   0xF104
#define RAS_ID                    0xF105
#define XJT_VERSION_ID            0xF106
#define R9_PWR_ID                 0xF107
#define SP2UART_A_ID              0xFD00
#define SP2UART_B_ID              0xFD01

// From openTX dataconstants.h : 
enum TelemetryUnit {
  UNIT_RAW,
  UNIT_VOLTS,
  UNIT_AMPS,
  UNIT_MILLIAMPS,
  UNIT_KTS,
  UNIT_METERS_PER_SECOND,
  UNIT_FEET_PER_SECOND,
  UNIT_KMH,
  UNIT_SPEED = UNIT_KMH,
  UNIT_MPH,
  UNIT_METERS,
  UNIT_DIST = UNIT_METERS,
  UNIT_FEET,
  UNIT_CELSIUS,
  UNIT_TEMPERATURE = UNIT_CELSIUS,
  UNIT_FAHRENHEIT,
  UNIT_PERCENT,
  UNIT_MAH,
  UNIT_WATTS,
  UNIT_MILLIWATTS,
  UNIT_DB,
  UNIT_RPMS,
  UNIT_G,
  UNIT_DEGREE,
  UNIT_RADIANS,
  UNIT_MILLILITERS,
  UNIT_FLOZ,
  UNIT_MILLILITERS_PER_MINUTE,
  UNIT_HERTZ,
  UNIT_MS,
  UNIT_US,
  UNIT_KM,
  UNIT_MAX = UNIT_KM,
  UNIT_SPARE5,
  UNIT_SPARE6,
  UNIT_SPARE7,
  UNIT_SPARE8,
  UNIT_SPARE9,
  UNIT_SPARE10,
  UNIT_HOURS,
  UNIT_MINUTES,
  UNIT_SECONDS,
  // FrSky format used for these fields, could be another format in the future
  UNIT_FIRST_VIRTUAL,
  UNIT_CELLS = UNIT_FIRST_VIRTUAL,
  UNIT_DATETIME,
  UNIT_GPS,
  UNIT_BITFIELD,
  UNIT_TEXT,
  // Internal units (not stored in sensor unit)
  UNIT_GPS_LONGITUDE,
  UNIT_GPS_LATITUDE,
  UNIT_DATETIME_YEAR,
  UNIT_DATETIME_DAY_MONTH,
  UNIT_DATETIME_HOUR_MIN,
  UNIT_DATETIME_SEC
};


void sport_init();
bool sport_update();

void _sport_isr();

void _sport_makeFrame();

void _sport_testCRC();

#endif