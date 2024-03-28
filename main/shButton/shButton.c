/*
 * shButton.cpp
 *
 *  Created on: Sep 20, 2020
 *      Author: tothpetiszilard
 */

#include "shButton.h"
#include "../MotorCtrl/MotorCtrl.h"

#include "../shInfo.h"
#if (10 < SH_SW_PATCH_VER)
#include "../Det/Det.h"
#endif

#ifndef ARDUINO
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"

#define digitalWrite(pin,val)	(gpio_set_level(pin,val))
#define millis()                (xTaskGetTickCount())
#define digitalRead(pin)	    (gpio_get_level(pin))

#define SHBUTTON_PIN_UP      (GPIO_NUM_13)
#define SHBUTTON_PIN_DOWN    (GPIO_NUM_3)

#define SHBUTTON_PRESSED      (0u)
#define SHBUTTON_RELEASED     (1u)

#else

#define SHBUTTON_PIN_UP      (13u)
#define SHBUTTON_PIN_DOWN    (3u)
#define SHBUTTON_PRESSED      (0u)
#define SHBUTTON_RELEASED     (1u)

#endif


typedef struct
{
	uint32_t lastPressed;
	uint32_t pressDuration;

}shButton_Hdl_tst;

typedef enum
{
	SHBUTTON_IDLE,
	SHBUTTON_UP,
	SHBUTTON_DOWN
}shButton_ActionType;

static uint8_t lastState_Up = SHBUTTON_RELEASED;
static uint8_t lastState_Down = SHBUTTON_RELEASED;
static shButton_ActionType pendingAct;

#ifndef ARDUINO
static void ShButton_Cyclic(void *pvParameters);
static TaskHandle_t ButtonTaskHdl = NULL;
#endif


void ShButton_Read(uint8_t ch_u8, uint8_t * val_pu8)
{
	if (ch_u8 == 0u)
	{
		*val_pu8 = digitalRead(SHBUTTON_PIN_UP);
	}
	else if (ch_u8 == 1u)
	{
		*val_pu8 = digitalRead(SHBUTTON_PIN_DOWN);
	}
}
#ifndef ARDUINO
static void ShButton_Cyclic(void *pvParameters)
{
	while(1)
#else
void ShButton_Cyclic(void)
#endif
{
	static uint32_t lastPressedTime = 0;
	uint8_t stateDown;
	uint8_t stateUp;
	uint32_t currTime = millis();
	if (pendingAct != SHBUTTON_IDLE)
	{
		uint8_t shutterState = MotorCtrl_GetState();
		if (pendingAct == SHBUTTON_UP)
		{
			//ShutterCtrl_Up();
			if (MOTORCTRL_UP != shutterState)
			{
				MotorCtrl_Up();
			}
			else 
			{
				// Nothing to do
			}
#if (10 < SH_SW_PATCH_VER)
	Det_PrintLog(DET_MODULEID_SHBUTTON, "Motor UP");
#endif
		}
		else if (pendingAct == SHBUTTON_DOWN)
		{
			
			if (MOTORCTRL_DOWN == shutterState)
			{
				MotorCtrl_Close();
			}
			else
			{
				MotorCtrl_Down();
			}
#if (10 < SH_SW_PATCH_VER)
	Det_PrintLog(DET_MODULEID_SHBUTTON, "Motor DOWN");
#endif
		}
		else
		{
			/*Invalid action */
		}
		pendingAct = SHBUTTON_IDLE;
	}
	stateUp = digitalRead(SHBUTTON_PIN_UP);
	stateDown = digitalRead(SHBUTTON_PIN_DOWN);
	if ((stateDown == SHBUTTON_PRESSED) && (stateUp == SHBUTTON_PRESSED))
	{
		// Both keys are pressed, invalid request
	}
	else
	{
		if ((lastState_Down == SHBUTTON_RELEASED) && (stateDown == SHBUTTON_PRESSED))
		{
			// Down key pressed
			#if (10 < SH_SW_PATCH_VER)
				Det_PrintLog(DET_MODULEID_SHBUTTON, "Down key pressed");
			#endif
			lastPressedTime = millis();
			lastState_Down = SHBUTTON_PRESSED;
		}
		else if ((lastState_Down == SHBUTTON_PRESSED) && (stateDown == SHBUTTON_RELEASED))
		{
			// Down key released
			#if (10 < SH_SW_PATCH_VER)
				Det_PrintLog(DET_MODULEID_SHBUTTON, "Down key released");
			#endif
			if ((lastPressedTime + 100u) <= currTime)
			{
				// Down key action
				//ShutterCtrl_Down();
				pendingAct = SHBUTTON_DOWN;
			}
#if (10 < SH_SW_PATCH_VER)
			else
			{
				Det_PrintLog(DET_MODULEID_SHBUTTON, "Down debounced");
			}
#endif
			lastState_Down = SHBUTTON_RELEASED;
		}
		else
		{
			lastState_Down = stateDown;
		}

		if ((lastState_Up == SHBUTTON_RELEASED) && (stateUp == SHBUTTON_PRESSED))
		{
			// Up key pressed
#if (10 < SH_SW_PATCH_VER)
			Det_PrintLog(DET_MODULEID_SHBUTTON, "Up key pressed");
#endif
			lastPressedTime = millis();
			lastState_Up = SHBUTTON_PRESSED;
		}
		else if ((lastState_Up == SHBUTTON_PRESSED) && (stateUp == SHBUTTON_RELEASED))
		{
			// Up key released
#if (10 < SH_SW_PATCH_VER)
			Det_PrintLog(DET_MODULEID_SHBUTTON, "Up key released");
#endif
			if ((lastPressedTime + 100u) <= currTime)
			{
				// Up key action
				//ShutterCtrl_Up();
				pendingAct = SHBUTTON_UP;
			}
#if (10 < SH_SW_PATCH_VER)
			else
			{
				Det_PrintLog(DET_MODULEID_SHBUTTON, "Up debounced");
			}
#endif
			lastState_Up = SHBUTTON_RELEASED;
		}
		else
		{
			lastState_Up = stateUp;
		}
	}
	#ifndef ARDUINO
	vTaskDelay(25u / portTICK_PERIOD_MS);
	}
	#endif
}


void ShButton_Init(void)
{
	#ifdef ARDUINO
	#if (SHBUTTON_RELEASED == 1) // PULLUP
	pinMode(SHBUTTON_PIN_UP, INPUT_PULLUP);
	pinMode(SHBUTTON_PIN_DOWN, INPUT_PULLUP);
    #else // PULLDOWN
	pinMode(SHBUTTON_PIN_UP, INPUT_PULLDOWN);
	pinMode(SHBUTTON_PIN_DOWN, INPUT_PULLDOWN);
	#endif // PULLUP/PULLDOWN
	
	#else // not ARDUINO
	#if (SHBUTTON_RELEASED == 1) // PULLUP
	gpio_config_t motorUpCfg =
	{
		.pin_bit_mask = GPIO_Pin_13,
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
		gpio_config_t motorDownCfg =
	{
		.pin_bit_mask = GPIO_Pin_3,
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_ENABLE,
		.pull_down_en = GPIO_PULLDOWN_DISABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	#else // PULLDOWN
		gpio_config_t motorUpCfg =
	{
		.pin_bit_mask = GPIO_Pin_13,
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
		gpio_config_t motorDownCfg =
	{
		.pin_bit_mask = GPIO_Pin_3,
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = GPIO_PULLUP_DISABLE,
		.pull_down_en = GPIO_PULLDOWN_ENABLE,
		.intr_type = GPIO_INTR_DISABLE
	};
	#endif // PULLUP/DOWN
	
	gpio_config(&motorUpCfg);
	gpio_config(&motorDownCfg);
	
	#endif // ARDUINO
	lastState_Up = digitalRead(SHBUTTON_PIN_UP);
	lastState_Down = digitalRead(SHBUTTON_PIN_DOWN);
	#ifndef ARDUINO
	xTaskCreate(ShButton_Cyclic, "shButton", 2048u, NULL, 5, &ButtonTaskHdl);
	#endif
}
