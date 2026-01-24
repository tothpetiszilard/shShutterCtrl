
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
//#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_spi_flash.h"

#include "WifiHdl.h"
#include "shCfg.h"
#include "update.h"
#include "ntp.h"
#include "shDiscovery.h"
#include "shCom.h"
#include "sunMoon.h"
#include "MotorCtrl.h"
#include "shScheduler.h"
#include "shStateMachine.h"
#include "shButton.h"

void app_main()
{
	static uint32_t secondCnt_u32 = 0;

    ShCfg_Init();

    MotorCtrl_Init();

    wifi_init_sta();

    Update_Init();
    NTP_Init();

	ShDiscovery_Init();
	ShCom_Init();
	ShSM_Init();

	ShButton_Init();

	SunMoon_Init(0, 47.412745, 19.174406);

	ShScheduler_Init();
	
	while(1)
	{
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		if (secondCnt_u32 < (3600u*24u))
		{
			secondCnt_u32++;
		}
		else
		{
			esp_restart();
		}
	}

}


