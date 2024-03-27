/*
 * shScheduler_Cfg.h
 *
 *  Created on: Jan 9, 2020
 *      Author: tothpetiszilard
 */

#ifndef SHSCHEDULER_CFG_H_
#define SHSCHEDULER_CFG_H_

#ifdef ARDUINO
#include "../shCfg/shCfg.h"
#include "TimeLib.h"
#include "../shStateMachine/ShSM.h"
#include "../MotorCtrl/MotorCtrl.h"
#include "../DayTime/DayTime.h"

#define SHSCHEDULER_WEEKDAY()					((timeDayOfWeek_t)weekday())
#define SHSCHEDULER_HOUR()						(hour())
/* 30 secs * 20 = 10 mins*/
#define SHSCHEDULER_CYCLECNT					((uint8_t)240u) /* 4 mins delay between adjustments */

#else
#include "shCfg.h"
#include "shStateMachine.h"
#include "MotorCtrl.h"
#include "DayTime.h"
#include "ntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define SHSCHEDULER_WEEKDAY()					((timeDayOfWeek_t)weekday(0))
#define SHSCHEDULER_HOUR()						(hour(0))

#define SHSCHEDULER_CYCLECNT					(60000u) /* 1 min (in ms) delay between adjustments */
#endif

#define SHSCHEDULER_CFG_CHID					((uint8_t)SHCFG_SCHTBL_CH)

#define SHSCHEDULER_READCFG(data_pu8)			ShCfg_Read(SHSCHEDULER_CFG_CHID, data_pu8)
#define SHSCHEDULER_GETSTATE()					(ShSM_GetState())
#define SHSCHEDULER_WINDOW_UP()					MotorCtrl_Up()
#define SHSCHEDULER_WINDOW_DOWN()				MotorCtrl_Down()
#define SHSCHEDULER_WINDOW_CLOSE()				MotorCtrl_Close()

typedef enum
{
	SHSCH_DONTCHANGE,
	SHSCH_UP,
	SHSCH_DOWN,
	SHSCH_CLOSE
} shSchedulerCmd_ten;

#endif /* SHSCHEDULER_CFG_H_ */
