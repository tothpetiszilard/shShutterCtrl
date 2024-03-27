/*
 * shDiscovery_Cfg.h
 *
 *  Created on: 2018. nov. 13.
 *      Author: tothpetiszilard
 */

#ifndef SHDISCOVERY_CFG_H_
#define SHDISCOVERY_CFG_H_

#include "shCfg.h"

#define SHDISCOVERY_SERACHER_MODE   (STD_ON)

#define SHDISCOVERY_LOCALPORT_U16	((uint16_t)(1111u))
#ifdef ARDUINO
#define SHDISCOVERY_MCAST_IP		(IPAddress(224,0,1,1))
#else
#define SHDISCOVERY_MCAST_IP		("224.0.1.1")
#endif
#define SHDISCOVERY_PACKET_SIZE		(50u)

#if (STD_ON == SHDISCOVERY_SERACHER_MODE)

//#include "../tempCtrl/tempCtrl.h"

#define SHDISCOVERY_GETPAIREDSN(data_pu8)	(ShCfg_Read(SHCFG_PAIREDNODE_CH,data_pu8))

#define SHDISCOVERY_RELAYIP_CB(ip)			//(TempCtrl_UpdateIP(ip))
#endif

#endif /* SHDISCOVERY_CFG_H_ */
