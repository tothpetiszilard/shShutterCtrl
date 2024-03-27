/*
 * MotorCtrl.cpp
 *
 *  Created on: May 7, 2020
 *      Author: tothpetiszilard
 */

#include "MotorCtrl.h"
#include "MotorCtrl_Cfg.h"
#include "Std_Types.h"

#include "../shInfo.h"
#if (10 < SH_SW_PATCH_VER)
#include "../Det/Det.h"
#endif

/*
typedef enum
{
	SHUTTER_UNKNOWN,
	SHUTTER_OPEN,
	SHUTTER_DOWN,
	SHUTTER_CLOSED
}MotorCtrl_ShutterState_ten;
*/

static MotorCtrl_Status_ten MotorCtrl_status_en;
//static MotorCtrl_ShutterState_ten MotorCtrl_shutter_en = SHUTTER_UNKNOWN;

static uint32_t lastChanged;
#ifndef ARDUINO
static TaskHandle_t MotorTaskHdl = NULL;
#endif
static uint16_t motorPosition_u16;

static uint16_t motorUpTime_u16 = MOTORCTRL_DRIVETIME_UP;
static uint16_t motorDownTime_u16 = MOTORCTRL_DRIVETIME_DOWN;
static uint16_t motorCloseTime_u16 = MOTORCTRL_DRIVETIME_CLOSE;

#ifndef ARDUINO
static void MotorCtrl_Cyclic(void *pvParameters);
#endif

void MotorCtrl_Init(void)
{
#ifdef ARDUINO
	pinMode(MOTORCTRL_PIN_UP, OUTPUT);
	pinMode(MOTORCTRL_PIN_DOWN,OUTPUT);
	digitalWrite(MOTORCTRL_PIN_UP,MOTORCTRL_OFF);
	digitalWrite(MOTORCTRL_PIN_DOWN,MOTORCTRL_OFF);
#else
	gpio_config_t motorEnCfg =
	{
		.pin_bit_mask = GPIO_Pin_4,
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&motorEnCfg);
	gpio_set_level(MOTORCTRL_PIN_nUP_DOWN, MOTORCTRL_OFF); // Switches to UPwards
		gpio_config_t motorDownCfg =
	{
		.pin_bit_mask = GPIO_Pin_5,
		.mode = GPIO_MODE_OUTPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	gpio_config(&motorDownCfg);
	gpio_set_level(MOTORCTRL_PIN_DOWN, MOTORCTRL_OFF); // Turns off SSR for mains
#endif	
	MotorCtrl_status_en = MOTORCTRL_IDLE;

	lastChanged = millis();
	// Read calibrations
	ShCfg_Read(SHCFG_MOTOR_UP_CH, (uint8_t *)&motorUpTime_u16);
	ShCfg_Read(SHCFG_MOTOR_DOWN_CH, (uint8_t *)&motorDownTime_u16);
	ShCfg_Read(SHCFG_MOTOR_CLOSE_CH, (uint8_t *)&motorCloseTime_u16);
	// Read actual shutter position
	if (E_OK == ShCfg_Read(SHCFG_MOTOR_CH, (uint8_t *)&motorPosition_u16))
	{
		// position is known
	}
	else
	{
		// Defaulting to top position
		motorPosition_u16 = motorCloseTime_u16; // lets assume it is totally down
		MotorCtrl_Up(); // roll it up

	}
#ifndef ARDUINO
	xTaskCreate(MotorCtrl_Cyclic, "MotorCtrl", 2048, NULL, 6, &MotorTaskHdl);
#endif
}

void MotorCtrl_Up(void)
{
	if ((MOTORCTRL_IDLE == MotorCtrl_status_en) && (MOTORCTRL_MOTORPOS_UP < motorPosition_u16))
	{
		digitalWrite(MOTORCTRL_PIN_ON,MOTORCTRL_ON); // Enable mains with SSR
		MotorCtrl_status_en = MOTORCTRL_UP;
		lastChanged = millis();
	}
	else if ((MOTORCTRL_DOWN == MotorCtrl_status_en) || (MOTORCTRL_CLOSE == MotorCtrl_status_en))
	{
		MotorCtrl_Stop();
		//lastChanged = millis();
		//MotorCtrl_shutter_en = SHUTTER_UNKNOWN;
	}
#if (10 < SH_SW_PATCH_VER)
	else
	{
		if (MOTORCTRL_MOTORPOS_UP >= motorPosition_u16)
		{
			Det_ReportError(DET_MODULEID_MOTORCTRL, "Up - Shutter already open");
		}
		else
		{
			Det_ReportError(DET_MODULEID_MOTORCTRL, "Up - Motor is not idle");
		}
	}
#endif
}

uint8_t MotorCtrl_GetState(void)
{
	uint8_t retVal_u8 = 255;
	if (MOTORCTRL_IDLE == MotorCtrl_status_en)
	{
		if (motorPosition_u16 == MOTORCTRL_MOTORPOS_UP)
		{
			retVal_u8 = (uint8_t)MOTORCTRL_UP;
		}
		else if ((motorPosition_u16 >= MOTORCTRL_MOTORPOS_DOWN) && (motorPosition_u16 < MOTORCTRL_MOTORPOS_CLOSE))
		{
			retVal_u8 = (uint8_t)MOTORCTRL_DOWN;
		}
		else if (motorPosition_u16 >= MOTORCTRL_MOTORPOS_CLOSE)
		{
			retVal_u8 = (uint8_t)MOTORCTRL_CLOSE;
		}
		else
		{
			// Not specified state
		}
	}
	return retVal_u8;
}

void MotorCtrl_GetDioLevel(uint8_t chId_u8,uint8_t * val_pu8)
{
	if (0u == chId_u8)
	{
		if (MOTORCTRL_DOWN == MotorCtrl_status_en)
		{
			*val_pu8 = 0u;
		}
		else
		{
			*val_pu8 = 1u;
		}
	}
	else if (1u == chId_u8)
	{
		if (MOTORCTRL_UP == MotorCtrl_status_en)
		{
			*val_pu8 = 0u;
		}
		else
		{
			*val_pu8 = 1u;
		}
	}
	else
	{
		// invalid channel
	}

}

void MotorCtrl_Stop(void)
{
#if (10 < SH_SW_PATCH_VER)
	char log[30];
#endif
	uint32_t currTime = millis();
	
	digitalWrite(MOTORCTRL_PIN_ON,MOTORCTRL_OFF); // Turn off mains voltage
	vTaskDelay(100 / portTICK_PERIOD_MS);
	digitalWrite(MOTORCTRL_PIN_nUP_DOWN, MOTORCTRL_OFF); // Set back relays position to UP 

	if ((MOTORCTRL_UP == MotorCtrl_status_en) && ((lastChanged + motorPosition_u16 + motorUpTime_u16) <= currTime))
	{
		// Calibration point reached
		motorPosition_u16 = 0u;
		ShCfg_Write(SHCFG_MOTOR_CH,(uint8_t *)&motorPosition_u16);
		#if (10 < SH_SW_PATCH_VER)
		Det_PrintLog(DET_MODULEID_MOTORCTRL, "Motor position: 0");
		#endif
	}
	else if ((MOTORCTRL_DOWN == MotorCtrl_status_en) || (MOTORCTRL_CLOSE== MotorCtrl_status_en))
	{
		if(currTime >= lastChanged)
		{
			motorPosition_u16 += (currTime - lastChanged);
			ShCfg_Write(SHCFG_MOTOR_CH,(uint8_t *)&motorPosition_u16);
			#if (10 < SH_SW_PATCH_VER)
			snprintf(log,30,"Motor position: %d",motorPosition_u16);
			Det_PrintLog(DET_MODULEID_MOTORCTRL,log);
			#endif
		}
		else
		{
			// overflow
			motorPosition_u16 += ((0xFFFFFFFFUL - lastChanged) + currTime);
			ShCfg_Write(SHCFG_MOTOR_CH,(uint8_t *)&motorPosition_u16);
			#if (10 < SH_SW_PATCH_VER)
			snprintf(log,30,"Motor position: %d",motorPosition_u16);
			Det_PrintLog(DET_MODULEID_MOTORCTRL,log);
			#endif
		}
	}
	else if (MOTORCTRL_UP == MotorCtrl_status_en)
	{
		// motor has running upwards, but before shutter reached open state, it was stopped
		if((currTime >= lastChanged) && (motorPosition_u16 >= (currTime - lastChanged)))
		{
			motorPosition_u16 -= (currTime - lastChanged);
			ShCfg_Write(SHCFG_MOTOR_CH,(uint8_t *)&motorPosition_u16);
			#if (10 < SH_SW_PATCH_VER)
			snprintf(log,30,"Motor position: %d",motorPosition_u16);
			Det_PrintLog(DET_MODULEID_MOTORCTRL,log);
			#endif
		}
		else if(currTime < lastChanged)
		{
			motorPosition_u16 -= ((0xFFFFFFFFUL - lastChanged) + currTime);
			ShCfg_Write(SHCFG_MOTOR_CH,(uint8_t *)&motorPosition_u16);
			#if (10 < SH_SW_PATCH_VER)
			snprintf(log,30,"Motor position: %d",motorPosition_u16);
			Det_PrintLog(DET_MODULEID_MOTORCTRL,log);
			#endif
		}
		else
		{
			// error
			#if (10 < SH_SW_PATCH_VER)
			Det_ReportError(DET_MODULEID_MOTORCTRL,"Motor position error");
			#endif

		}
	}
	else
	{
		// idle
	}
	MotorCtrl_status_en = MOTORCTRL_IDLE;
}

void MotorCtrl_Down(void)
{
	if ((MOTORCTRL_IDLE == MotorCtrl_status_en) && (MOTORCTRL_MOTORPOS_DOWN > motorPosition_u16))
	{
		digitalWrite(MOTORCTRL_PIN_nUP_DOWN, MOTORCTRL_ON); // Switch to the DOWN coil with relay 
		vTaskDelay(100 / portTICK_PERIOD_MS);
		digitalWrite(MOTORCTRL_PIN_ON,MOTORCTRL_ON); // Turn ON mains voltage via SSR
		MotorCtrl_status_en = MOTORCTRL_DOWN;
		lastChanged = millis();
	}
	else if (MOTORCTRL_UP == MotorCtrl_status_en)
	{
		MotorCtrl_Stop();
		//lastChanged = millis();
		//MotorCtrl_shutter_en = SHUTTER_UNKNOWN;
	}
#if (10 < SH_SW_PATCH_VER)
	else
	{
		if (MOTORCTRL_MOTORPOS_DOWN == motorPosition_u16)
		{
			Det_ReportError(DET_MODULEID_MOTORCTRL, "Down - Shutter already down");
		}
		else if (MOTORCTRL_MOTORPOS_DOWN < motorPosition_u16)
		{
			Det_ReportError(DET_MODULEID_MOTORCTRL, "Down - Shutter already lower");
		}
		else
		{
			Det_ReportError(DET_MODULEID_MOTORCTRL, "Down - Motor is not idle");
		}
	}
#endif
}

void MotorCtrl_Close(void)
{
	if ((MOTORCTRL_IDLE == MotorCtrl_status_en) && (MOTORCTRL_MOTORPOS_CLOSE > motorPosition_u16))
	{
		digitalWrite(MOTORCTRL_PIN_nUP_DOWN, MOTORCTRL_ON); // Switch to the DOWN coil with relay 
		vTaskDelay(100 / portTICK_PERIOD_MS);
		digitalWrite(MOTORCTRL_PIN_ON,MOTORCTRL_ON); // Turns mains voltage ON
		MotorCtrl_status_en = MOTORCTRL_CLOSE;
		lastChanged = millis();
	}
	else if (MOTORCTRL_UP == MotorCtrl_status_en)
	{
		MotorCtrl_Stop();
		//lastChanged = millis();
		//MotorCtrl_shutter_en = SHUTTER_UNKNOWN;
	}
#if (10 < SH_SW_PATCH_VER)
	else
	{
		if (MOTORCTRL_MOTORPOS_CLOSE <= motorPosition_u16)
		{
			Det_ReportError(DET_MODULEID_MOTORCTRL, "Close - Shutter already closed");
		}
		else
		{
			Det_ReportError(DET_MODULEID_MOTORCTRL, "Close - Motor is not idle");
		}
	}
#endif
}

#ifndef ARDUINO
static void MotorCtrl_Cyclic(void *pvParameters)
#else
void MotorCtrl_Cyclic(void)
#endif
{
	#ifndef ARDUINO
	while(1u)
	{
	#endif
	uint32_t currTime = (millis());
	if ((MOTORCTRL_DOWN == MotorCtrl_status_en) && ((lastChanged + (motorDownTime_u16 - motorPosition_u16)) <= currTime ))
	{
		//MotorCtrl_shutter_en = SHUTTER_DOWN;
		MotorCtrl_Stop();
#if (10 < SH_SW_PATCH_VER)
		Det_PrintLog(DET_MODULEID_MOTORCTRL, "Shutter is down");
#endif

	}
	else if ((MOTORCTRL_UP == MotorCtrl_status_en) && ((lastChanged + (motorPosition_u16 + motorUpTime_u16)) <= currTime ))
	{
		//MotorCtrl_shutter_en = SHUTTER_OPEN;
		MotorCtrl_Stop();
#if (10 < SH_SW_PATCH_VER)
		Det_PrintLog(DET_MODULEID_MOTORCTRL, "Shutter is open");
#endif

	}
	else if ((MOTORCTRL_CLOSE == MotorCtrl_status_en) && ((lastChanged + (motorCloseTime_u16 - motorPosition_u16)) <= currTime ))
	{
		//MotorCtrl_shutter_en = SHUTTER_CLOSED;
		MotorCtrl_Stop();
#if (10 < SH_SW_PATCH_VER)
		Det_PrintLog(DET_MODULEID_MOTORCTRL, "Shutter is closed");
#endif
	}
	else
	{

	}
	#ifndef ARDUINO
	//esp_task_wdt_reset(); // Feed watchdog
	vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	#endif
}
