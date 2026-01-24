/*
 * shCfg.h
 *
 *  Created on: Oct 29, 2019
 *      Author: tothpetiszilard
 */

#ifndef SHCFG_H_
#define SHCFG_H_

#ifdef __cplusplus

extern "C" {
#endif // __cplusplus

#include "../../Std_Types.h"

#define SHCFG_SSID_CH	        0u
#define SHCFG_PASSWD_CH	        1u
#define SHCFG_SCHTBL_CH	        2u
#define SHCFG_MOTOR_CH	        3u
#define SHCFG_PAIREDNODE_CH	    4u
#define SHCFG_MOTOR_UP_CH	    64u
#define SHCFG_MOTOR_DOWN_CH	    65u
#define SHCFG_MOTOR_CLOSE_CH    66u
#define SHCFG_SMSTATE_CH	    128u
#define SHCFG_NTPSRV_CH	        192u
#define SHCFG_NTPTZ_CH	        193u
#define SHCFG_DETIP_CH	        254u
#define SHCFG_IDENTDATA_CH	    255u


extern StdReturnType ShCfg_Init(void);

extern const char * ShCfg_GetCharArray(uint8_t chId_u8);

extern uint8_t ShCfg_GetSize(uint8_t chId_u8);

extern StdReturnType ShCfg_Read(uint8_t ch_u8, uint8_t * data_pu8);

extern StdReturnType ShCfg_Write(uint8_t ch_u8, uint8_t * data_pu8);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* SHCFG_H_ */
