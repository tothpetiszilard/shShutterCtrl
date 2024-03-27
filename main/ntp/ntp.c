/*
 * ntp.c
 *
 *  Created on: Dec 12, 2020
 *      Author: tothpetiszilard
 */


#include "ntp.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include <string.h>
#include <sys/time.h>
#include <stdlib.h>
#include "lwip/apps/sntp.h"
#include "shCfg.h"
#include "esp_log.h"


void NTP_Init(void)
{
	char timezone_ach[30] = "CET-1CEST,M3.5.0,M10.5.0/3";
	char ntpSrv_ach[30] = "192.168.0.1";
#if (0 == SNTP_SERVER_DNS)
	ip_addr_t ntpSrv_ip;
#endif
	sntp_stop();
	(void)ShCfg_Read(SHCFG_NTPSRV_CH,(uint8_t *)ntpSrv_ach);
	#if SNTP_SERVER_DNS
		sntp_setservername(0u, ntpSrv_ach);
	#else
		ipaddr_aton(ntpSrv_ach,&ntpSrv_ip);
		sntp_setserver(0u,&ntpSrv_ip);
	#endif /* SNTP_SERVER_DNS */
	(void)ShCfg_Read(SHCFG_NTPTZ_CH,(uint8_t *)timezone_ach);
	sntp_init();


	time_t now = 0;
	struct tm timeinfo = { 0 };
	int retry = 0;
	const int retry_count = 10;
	while (timeinfo.tm_year < (2016 - 1900) && ++retry < retry_count)
	{
		//ESP_LOGI("NTP", "Waiting for system time to be set... (%d/%d)", retry, retry_count);
		vTaskDelay(1000 / portTICK_PERIOD_MS);
		time(&now);
		localtime_r(&now, &timeinfo);
	}

	setenv("TZ", timezone_ach, 1);
	tzset();
}

time_t now(void)
{
	return time(NULL);
}

timeDayOfWeek_t weekday(time_t t)
{
	timeDayOfWeek_t retVal;
	struct tm tElements;
	time_t currTime;
	if (0 == t)
	{
		currTime = time(NULL);
	}
	else
	{
		currTime = t;
	}
	localtime_r(&currTime, &tElements);
	retVal = (tElements).tm_wday;
	return retVal;
}

uint8_t hour(time_t t)
{
	uint8_t retVal = 0;
	struct tm tElements;
	time_t currTime;
	if (0 == t)
	{
		currTime = time(NULL);
	}
	else
	{
		currTime = t;
	}
	localtime_r(&currTime, &tElements);
	retVal = (tElements).tm_hour;
	return retVal;
}
uint8_t minute(time_t t)
{
	uint8_t retVal = 0;
	struct tm tElements;
	time_t currTime;
	if (0 == t)
	{
		currTime = time(NULL);
	}
	else
	{
		currTime = t;
	}
	localtime_r(&currTime, &tElements);
	retVal = (tElements).tm_min;
	return retVal;
}
uint8_t second(time_t t)
{
	uint8_t retVal = 0;
	struct tm tElements;
	time_t currTime;
	if (0 == t)
	{
		currTime = time(NULL);
	}
	else
	{
		currTime = t;
	}
	localtime_r(&currTime, &tElements);
	retVal = (tElements).tm_sec;
	return retVal;
}

uint16_t year(time_t t)
{
	uint16_t retVal = 0;
	struct tm tElements;
	time_t currTime;
	if (0 == t)
	{
		currTime = time(NULL);
	}
	else
	{
		currTime = t;
	}
	localtime_r(&currTime, &tElements);
	retVal = 1900 + (tElements).tm_year; //years since 1900
	return retVal;
}
uint8_t month(time_t t)
{
	uint8_t retVal = 0;
	struct tm tElements;
	time_t currTime;
	if (0 == t)
	{
		currTime = time(NULL);
	}
	else
	{
		currTime = t;
	}
	localtime_r(&currTime, &tElements);
	retVal = 1 + (tElements).tm_mon; //month of year [0,11]
	return retVal;
}
uint8_t day(time_t t)
{
	uint8_t retVal = 0;
	struct tm tElements;
	time_t currTime;
	if (0 == t)
	{
		currTime = time(NULL);
	}
	else
	{
		currTime = t;
	}
	localtime_r(&currTime, &tElements);
	retVal = (tElements).tm_mday; //day of month [1,31]
	return retVal;
}

uint32_t NTP_getUTCOffset(void)
{
	return (uint32_t)((long)(-1) * _timezone);
}

