/*
 * shSchduler.cpp
 *
 *  Created on: Jan 9, 2020
 *      Author: tothpetiszilard
 */

#include "shScheduler.h"
#include "shScheduler_Cfg.h"

#include "../shInfo.h"
#if (10 < SH_SW_PATCH_VER)
#include "../Det/Det.h"
#endif

static uint8_t schTable_aau8[8u][24u]; /* (7+1) days * 24 hours = 192 bytes */

static inline shSchedulerCmd_ten getTableData_u8(timeDayOfWeek_t day, uint8_t hour_u8);
static inline void ShScheduler_DoCmd(shSchedulerCmd_ten cmd);
#ifndef ARDUINO
static void ShScheduler_Cyclic(void *pvParameters);
#endif
	
void ShScheduler_Init(void)
{
	uint8_t day_u8 = 0u;
	uint8_t hour_u8 = 0u;
	if (E_OK == SHSCHEDULER_READCFG((uint8_t*)schTable_aau8))
	{
		/* Read successful */
	}
	else
	{
#if (10 < SH_SW_PATCH_VER)
		Det_ReportError(DET_MODULEID_SHSCHEDULER, "Cfg read failed");
#endif
		/* Read failed, set to dont touch */
		for (day_u8 = 0u; day_u8 < 8u; day_u8++)
		{
			for (hour_u8 = 0u; hour_u8 < 24u; ++hour_u8)
			{
				schTable_aau8[day_u8][hour_u8] = SHSCH_DONTCHANGE;
			}
		}
	}
	#ifndef ARDUINO
	xTaskCreate(ShScheduler_Cyclic, "ShScheduler", 2048, NULL, 5, NULL);
	#endif
}


#ifndef ARDUINO
static void ShScheduler_Cyclic(void *pvParameters)
#else
void ShScheduler_Cyclic(void)
#endif
{
	static uint8_t lastHour_u8 = 0u;
	static DayTime_RetVal_ten lastSun_en = DAYTIME_NOT_AVAILABLE;
	timeDayOfWeek_t currentDay = dowInvalid;
	uint8_t currentHour_u8 = 0u;
	DayTime_RetVal_ten currentSun_en = DAYTIME_NOT_AVAILABLE;
	ShSM_States_ten currState_en = MANUAL_MODE;
	shSchedulerCmd_ten command_en = SHSCH_DONTCHANGE;
	#ifndef ARDUINO
	while(1u)
	#else
	static uint8_t cycleCnt_u8 = 0u;
	if (SHSCHEDULER_CYCLECNT <= cycleCnt_u8)
	#endif
	{
		if (E_OK == SHSCHEDULER_READCFG((uint8_t*)schTable_aau8))
		{
			/* Read successful */
			currState_en = SHSCHEDULER_GETSTATE();
#if (10 < SH_SW_PATCH_VER)
			switch(currState_en)
			{
				case SUNSETD_MODE:
					Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Sunset + Daily mode");
					break;
				case DAILY_MODE:
					Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Daily mode");
					break;
				case SUNSETW_MODE:
					Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Sunset + Weekly mode");
					break;
				case WEEKLY_MODE:
					Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Weekly mode");
					break;
				default:
					Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Invalid mode");
					break;
			}
			Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Schedule time, cfg OK");
#endif
			if ((WEEKLY_MODE == currState_en) || (SUNSETW_MODE == currState_en))
			{
				currentDay = SHSCHEDULER_WEEKDAY();
				currentHour_u8 = SHSCHEDULER_HOUR();
				if (SUNSETW_MODE == currState_en)
				{
					currentSun_en = DayTime_Get();
#if (10 < SH_SW_PATCH_VER)
					switch (currentSun_en)
					{
					case DAYTIME_MOONLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Current: Moonlight");
					break;
					case DAYTIME_SUNLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Current: Sunlight");
					break;
					case DAYTIME_NOT_AVAILABLE:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Current: N/A");
					break;
					default:
					break;
					}
					switch (lastSun_en)
					{
					case DAYTIME_MOONLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Last: Moonlight");
					break;
					case DAYTIME_SUNLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Last: Sunlight");
					break;
					case DAYTIME_NOT_AVAILABLE:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Last: N/A");
					break;
					default:
					break;
					}
#endif
					if (currentSun_en != lastSun_en)
					{
						if (currentSun_en == DAYTIME_MOONLIGHT)
						{
#if (10 < SH_SW_PATCH_VER)
							Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Daylight->Moonlight");
#endif
							command_en = SHSCH_DOWN;
							ShScheduler_DoCmd(command_en);
						}
#if (10 < SH_SW_PATCH_VER)
						else if (currentSun_en == DAYTIME_SUNLIGHT)
						{
							Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Moonlight->Daylight");
						}
						else
						{
							Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Daytime N/A");
						}
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Weekly mode - Sun changed");
#endif
						lastSun_en = currentSun_en;
					}
				}

				if (currentHour_u8 != lastHour_u8)
				{
#if (10 < SH_SW_PATCH_VER)
					Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Weekly mode - Hour changed");
#endif
					command_en = getTableData_u8(currentDay, currentHour_u8);
					ShScheduler_DoCmd(command_en);
					lastHour_u8 = currentHour_u8;
				}
			}
			else if ((DAILY_MODE == currState_en) || (SUNSETD_MODE == currState_en))
			{
				currentDay = dowInvalid;
				currentHour_u8 = SHSCHEDULER_HOUR();
				if (SUNSETD_MODE == currState_en)
				{
					currentSun_en = DayTime_Get();
#if (10 < SH_SW_PATCH_VER)
					switch (currentSun_en)
					{
					case DAYTIME_MOONLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Current: Moonlight");
					break;
					case DAYTIME_SUNLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Current: Sunlight");
					break;
					case DAYTIME_NOT_AVAILABLE:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Current: N/A");
					break;
					default:
					break;
					}
					switch (lastSun_en)
					{
					case DAYTIME_MOONLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Last: Moonlight");
					break;
					case DAYTIME_SUNLIGHT:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Last: Sunlight");
					break;
					case DAYTIME_NOT_AVAILABLE:
						Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Last: N/A");
					break;
					default:
					break;
					}
#endif
					if (currentSun_en != lastSun_en)
					{
						if (currentSun_en == DAYTIME_MOONLIGHT)
						{
#if (10 < SH_SW_PATCH_VER)
#endif
							command_en = SHSCH_DOWN;
							ShScheduler_DoCmd(command_en);
						}
#if (10 < SH_SW_PATCH_VER)
						else if (currentSun_en == DAYTIME_SUNLIGHT)
						{
						}
						else
						{
						}
#endif
						lastSun_en = currentSun_en;
					}
				}
				if (currentHour_u8 != lastHour_u8)
				{
#if (10 < SH_SW_PATCH_VER)
					Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Daily mode - Hour changed");
#endif
					command_en = getTableData_u8(currentDay, currentHour_u8);
					ShScheduler_DoCmd(command_en);
					lastHour_u8 = currentHour_u8;
				}
			}
			else
			{
				/* Manual mode */
#if (10 < SH_SW_PATCH_VER)
				Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Manual mode - No action");
#endif
			}
		}
		else
		{
#if (10 < SH_SW_PATCH_VER)
			Det_ReportError(DET_MODULEID_SHSCHEDULER, "Sch table read failed");
#endif
			/* Schedule table read error */
			if (MANUAL_MODE != currState_en)
			{
				/* manual mode is not active, no change */
				currentDay = dowInvalid;
				currentHour_u8 = SHSCHEDULER_HOUR();
				command_en = getTableData_u8(currentDay, currentHour_u8);
				if (currentHour_u8 != lastHour_u8)
				{
					ShScheduler_DoCmd(command_en);
					lastHour_u8 = currentHour_u8;
				}
			}
		}
		#ifndef ARDUINO
		vTaskDelay(SHSCHEDULER_CYCLECNT / portTICK_PERIOD_MS);
		#else
		cycleCnt_u8 = 0u;
	}
	else
	{
		cycleCnt_u8++;
		#endif
	}

}

static inline void ShScheduler_DoCmd(shSchedulerCmd_ten cmd_en)
{
	switch (cmd_en)
	{
		case SHSCH_UP:
#if (10 < SH_SW_PATCH_VER)
			Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Motor UP");
#endif
			MotorCtrl_Up();
			break;
		case SHSCH_DOWN:
#if (10 < SH_SW_PATCH_VER)
			Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Motor DOWN");
#endif
			MotorCtrl_Down();
			break;
		case SHSCH_CLOSE:
#if (10 < SH_SW_PATCH_VER)
			Det_PrintLog(DET_MODULEID_SHSCHEDULER, "Motor CLOSE");
#endif
			MotorCtrl_Close();
			break;
		case SHSCH_DONTCHANGE:
			break;
		default:
#if (10 < SH_SW_PATCH_VER)
			Det_ReportError(DET_MODULEID_SHSCHEDULER, "Motor INVALID");
#endif
			break;
	}
}

static inline shSchedulerCmd_ten getTableData_u8(timeDayOfWeek_t day, uint8_t hour_u8)
{
	shSchedulerCmd_ten retVal_en = SHSCH_DONTCHANGE;
	switch (day)
	{
	case dowMonday:
#ifdef DEBUG
		Serial.print("Monday ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[1u][hour_u8]);
		break;
	case dowTuesday:
#ifdef DEBUG
		Serial.print("Tuesday ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[2u][hour_u8]);
		break;
	case dowWednesday:
#ifdef DEBUG
		Serial.print("Wednesday ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[3u][hour_u8]);
		break;
	case dowThursday:
#ifdef DEBUG
		Serial.print("Thursday ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[4u][hour_u8]);
		break;
	case dowFriday:
#ifdef DEBUG
		Serial.print("Friday ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[5u][hour_u8]);
		break;
	case dowSaturday:
#ifdef DEBUG
		Serial.print("Saturday ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[6u][hour_u8]);
		break;
	case dowSunday:
#ifdef DEBUG
		Serial.print("Sunday ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[7u][hour_u8]);
		break;
	default:
#ifdef DEBUG
		Serial.print("Daily mode ");
#endif
		retVal_en = ((shSchedulerCmd_ten)schTable_aau8[0u][hour_u8]);
		break;
	}
#ifdef DEBUG
	Serial.print(hour_u8);
	Serial.print(" h :");
	Serial.println(retVal_en);
#endif
	return retVal_en;
}
