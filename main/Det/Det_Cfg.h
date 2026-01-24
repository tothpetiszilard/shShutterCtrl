/*
 * Det_Cfg.h
 *
 *  Created on: Aug 31, 2020
 *      Author: tothpetiszilard
 */

#ifndef DET_CFG_H_
#define DET_CFG_H_

#ifdef ARDUINO

#define DET_MODULEID_MAIN				0u
#define DET_MODULEID_NTP				1u
#define DET_MODULEID_DAYTIME			2u
#define DET_MODULEID_SHCFG				3u
#define DET_MODULEID_SHCOM				4u
#define DET_MODULEID_SHDISCOVERY		5u
#define DET_MODULEID_SHSTATEMACHINE		6u
#define DET_MODULEID_SHSCHEDULER		7u
#define DET_MODULEID_WIFIHDL			8u
#define DET_MODULEID_MOTORCTRL			9u
#define DET_MODULEID_DET 				10u

#else

#define DET_MODULEID_MAIN				"MAIN"
#define DET_MODULEID_NTP				"NTP"
#define DET_MODULEID_DAYTIME			"DAYT"
#define DET_MODULEID_SHCFG				"SHCFG"
#define DET_MODULEID_SHCOM				"SHCOM"
#define DET_MODULEID_SHDISCOVERY		"SHDISC"
#define DET_MODULEID_SHSTATEMACHINE		"SHSM"
#define DET_MODULEID_SHSCHEDULER		"SHSCH"
#define DET_MODULEID_WIFIHDL			"WIFIHDL"
#define DET_MODULEID_MOTORCTRL			"MOTOR"
#define DET_MODULEID_DET 				"DET"

#endif

#endif /* DET_CFG_H_ */
