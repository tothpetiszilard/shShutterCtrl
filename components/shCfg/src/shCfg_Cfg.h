/*
 * shCfg_Cfg.h
 *
 *  Created on: Oct 29, 2019
 *      Author: tothpetiszilard
 */

#ifndef SHCFG_CFG_H_
#define SHCFG_CFG_H_

#include "shCfg.h"
#include "esp_littlefs.h"

typedef struct
{
	const uint8_t chId_cu8;
	const uint8_t size_cu8;
} shCfgEntry_tst;

#define SHCFG_CONFIGSIZE	13u

const shCfgEntry_tst shCfg_ConfigTable_cast[SHCFG_CONFIGSIZE] =
{
		{
				.chId_cu8 = SHCFG_SSID_CH, // "SSID"
				.size_cu8 = 32u // "TheCityOfLight"
		},
		{
				.chId_cu8 = SHCFG_PASSWD_CH, // "password"
				.size_cu8 = 13u // "mexikomabeka\0"
		},
		{
				.chId_cu8 = SHCFG_PAIREDNODE_CH, // "paired relay sn"
				.size_cu8 = 2u // "xxxx"
		},
		{
				.chId_cu8 = SHCFG_SCHTBL_CH, // "schedule table"
				.size_cu8 = 192u // (7+1) days * 24 hours = 192 bytes
		},
		{
				.chId_cu8 = SHCFG_MOTOR_CH, // shutter actual state
				.size_cu8 = 2u // 16 bit timestamp
		},
		{
				.chId_cu8 = SHCFG_MOTOR_UP_CH, // extra drivetime for upwards dir
				.size_cu8 = 2u // "xxxx"
		},
		{
				.chId_cu8 = SHCFG_MOTOR_DOWN_CH, // drivetime for downwards dir
				.size_cu8 = 2u // "xxxx"
		},
		{
				.chId_cu8 = SHCFG_MOTOR_CLOSE_CH, // drivetime for full close
				.size_cu8 = 2u // "xxxx"
		},
		{
				.chId_cu8 = SHCFG_SMSTATE_CH, //shSM actual state
				.size_cu8 = 1u // 8bit
		},
		{
				.chId_cu8 = SHCFG_NTPSRV_CH, // "NTP server"
				.size_cu8 = 30u // "xxxx"
		},
		{
				.chId_cu8 = SHCFG_NTPTZ_CH, // "NTP timezone"
				.size_cu8 = 30u // "CET-1CEST,M3.5.0,M10.5.0/3"
		},
		{
				.chId_cu8 = SHCFG_DETIP_CH, // "Det IP address"
				.size_cu8 = 4u    // 32 bit
		},
		{
				.chId_cu8 = SHCFG_IDENTDATA_CH, // "DeviceID"
				.size_cu8 = 4u    // 32 bit
		}
};

#endif /* SHCFG_CFG_H_ */
