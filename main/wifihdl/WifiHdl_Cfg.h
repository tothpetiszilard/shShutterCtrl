/*
 * SSC_Wifi_Cfg.h
 *
 *  Created on: 2017. dec. 9.
 *      Author: tothpetiszilard
 */

#ifndef WIFIHDL_CFG_H_
#define WIFIHDL_CFG_H_


#include "shCfg.h"

#define WIFIHDL_GETSSID(data_pu8)			(ShCfg_Read((uint8_t)SHCFG_SSID_CH, data_pu8))
#define WIFIHDL_GETPWD(data_pu8)			(ShCfg_Read((uint8_t)SHCFG_PASSWD_CH,data_pu8))
#define WIFIHDL_GETDEVTYPE(data_pu8)		(ShCfg_Read((uint8_t)SHCFG_IDENTDATA_CH,data_pu8))

#define WIFIHDL_CONN_TIMEOUT (500u) // timeout in 100ms

#endif /* WIFIHDL_CFG_H_ */
