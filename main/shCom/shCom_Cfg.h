/*
 * shCom_Cfg.h
 *
 *  Created on: 2018. nov. 13.
 *      Author: tothpetiszilard
 */

#ifndef SHCOM_CFG_H_
#define SHCOM_CFG_H_

#include "../shInfo.h"
#ifdef ARDUINO
#include "../shCfg/shCfg.h"
#include "../shStateMachine/shStateMachine.h"
#else
#include "shCfg.h"
#include "shStateMachine.h"
#endif

#if ((DEVSUBTYPE == 'W') && (DEVTYPE == 'A'))
#include "MotorCtrl.h"
#endif

#if ((DEVSUBTYPE == 'L') && (DEVTYPE == 'D'))
#include "../DspGfx/DspGfx.h"
#endif

#define SHCOM_UDP_MINSIZE							(2u)
#define SHCOM_UDP_MAXSIZE							(1000u)
#define SHCOM_UDP_PORT								(3333u)

#define SHCOM_GETHIGHBYTE(val_u16)					((uint8_t)((val_u16 & (uint16_t)0xFF00) >> (uint16_t)8u))
#define SHCOM_GETLOWBYTE(val_u16)					((uint8_t)(val_u16 & (uint16_t)0xFF))
#define SHCOM_GETWORD(high_u8,low_u8)				((uint16_t)(((uint16_t)high_u8 << (uint16_t)8u) | ((uint16_t)low_u8)))

#define SHCOM_GETSTATE()							((uint8_t)ShSM_GetState())
#define SHCOM_SETSTATE(mode_u8)						(ShSM_SetState((ShSM_States_ten)mode_u8))

#if ((DEVSUBTYPE == 'W') && (DEVTYPE == 'A'))
#define SHCOM_GETDIO_PERCH(chId_u8,val_pu8)			(MotorCtrl_GetDioLevel(chId_u8,val_pu8))
#define SHCOM_WINDOW_UP()							(MotorCtrl_Up())
#define SHCOM_WINDOW_DOWN()							(MotorCtrl_Down())
#define SHCOM_WINDOW_CLOSE()						(MotorCtrl_Close())
#define SHCOM_WINDOW_STOP()							(MotorCtrl_Stop())
#define SHCOM_GET_WINDOWSTATE()                     (MotorCtrl_GetState())
#endif
#if ((DEVSUBTYPE == 'L') && (DEVTYPE == 'D'))
#define SHCOM_WRITEGFX(data_pu8,len_u16)			(DspGfx_SetPixels(data_pu8,len_u16))
#endif

#define SHCOM_READCFG(ch_u8, data_pu8)				(ShCfg_Read(ch_u8, data_pu8))
#define SHCOM_WRITECFG(ch_u8, data_pu8)				(ShCfg_Write(ch_u8, data_pu8))

#define SHCOM_GETSIZECFG(ch_u8)						(ShCfg_GetSize(ch_u8))

#endif /* SHCOM_CFG_H_ */
