/*
 * SunRise.cpp
 *
 *  Created on: 2018. nov. 10.
 *      Author: tothpetiszilard
 */

#ifdef ARDUINO

#include "ArduinoJson.h"
#include <ESP8266HTTPClient.h>
#include "TimeLib.h"
#include "../NTP/NTP.h"
#include "DayTime.h"

#include "../shInfo.h"
#if (10 < SH_SW_PATCH_VER)
#include "../Det/Det.h"
#endif

#define DAYTIME_DEBOUNCE_MS      (60000u) // Minimum 1 minute between updates

static bool DayTime_dataValid_b = false;
static uint8_t sunRiseHour_u8;
static uint8_t sunRiseMinute_u8;
static uint8_t sunSetHour_u8;
static uint8_t sunSetMinute_u8;
static uint32 lastUpdate = 0;

void DayTime_Cyclic(void)
{
	uint32 currentTime = millis();
	if ((false == DayTime_dataValid_b) && (true == NTP_IsTimeValid()) && (currentTime > (lastUpdate + DAYTIME_DEBOUNCE_MS)))
	{
		HTTPClient http;
		uint32_t httpCode_u32 = 0;
		StaticJsonDocument<1000> jsonBuffer;
		String httpData;
		String DayTime_sunRise_s;
		String DayTime_sunSet_s;
		const char * status_cc;
		const char * sunRise_cc;
		const char * sunSet_cc;

		http.begin("http://api.sunrise-sunset.org/json?lat=47.404824&lng=19.169337&date=today");
		httpCode_u32 = http.GET();	//Send the request
		if (httpCode_u32 == HTTP_CODE_OK)
		{
			httpData = http.getString();
			DeserializationError error = deserializeJson(jsonBuffer, httpData); //Get the request response payload
			if (false == error)
			{
				status_cc = jsonBuffer["status"];
				if (0 == strcmp(status_cc,"OK"))
				{
					sunRise_cc = jsonBuffer["results"]["sunrise"];
					DayTime_sunRise_s = sunRise_cc;
					sunSet_cc = jsonBuffer["results"]["sunset"];
					DayTime_sunSet_s = sunSet_cc;
					sunRiseHour_u8 = (uint8_t)(DayTime_sunRise_s.substring(0u, DayTime_sunRise_s.indexOf(':'))).toInt();
					sunRiseMinute_u8 = (uint8_t)(DayTime_sunRise_s.substring(DayTime_sunRise_s.indexOf(':')+1u, DayTime_sunRise_s.lastIndexOf(':'))).toInt();
					sunSetHour_u8 = (uint8_t)(DayTime_sunSet_s.substring(0u, DayTime_sunSet_s.indexOf(':'))).toInt();
					if (true == (DayTime_sunSet_s.substring(DayTime_sunSet_s.indexOf(' ')+1u).equals("PM")))
					{
						sunSetHour_u8 += 12u;
					}
					sunSetMinute_u8 = (uint8_t)(DayTime_sunSet_s.substring(DayTime_sunSet_s.indexOf(':')+1u, DayTime_sunSet_s.lastIndexOf(':'))).toInt();
					/* TimeZone correction */
					sunSetHour_u8 += (uint8_t)(NTP_getUTCOffset()/(uint32_t)SECS_PER_HOUR);
					sunRiseHour_u8 += (uint8_t)(NTP_getUTCOffset()/(uint32_t)SECS_PER_HOUR);
					DayTime_dataValid_b = true;
					lastUpdate = millis();
#if (10 < SH_SW_PATCH_VER)
				Det_PrintLog(DET_MODULEID_DAYTIME, "Daytime data updated");

				}
				else
				{
					//DayTime_dataValid_b = false;
					Det_ReportError(DET_MODULEID_DAYTIME, "Json status not ok");
#endif
				}
			}
#if (10 < SH_SW_PATCH_VER)
			else
			{

				//DayTime_dataValid_b = false;
				Det_ReportError(DET_MODULEID_DAYTIME, "Json failed");
			}
#endif
		}
#if (10 < SH_SW_PATCH_VER)
		else
		{
			//DayTime_dataValid_b = false;
			Det_ReportError(DET_MODULEID_DAYTIME, "HTTP failed");
		}
#endif
		http.end();   //Close connection
	}
}

void DayTime_Update(void)
{
	DayTime_dataValid_b = false;
}

DayTime_RetVal_ten DayTime_Get(void)
{
	DayTime_RetVal_ten retVal_en = DAYTIME_NOT_AVAILABLE;

	uint8_t currentHour_u8;
	uint8_t currentMinute_u8;
#if (10 < SH_SW_PATCH_VER)
	char log[64];
#endif
	if ((true == DayTime_dataValid_b) && (true == NTP_IsTimeValid()))
	{
		currentHour_u8 = (uint8_t)hour();
		currentMinute_u8 = (uint8_t)minute();
#if (10 < SH_SW_PATCH_VER)
		snprintf(log,63,"T: %d:%d SR: %d:%d SS: %d:%d",currentHour_u8,currentMinute_u8,sunRiseHour_u8,sunRiseMinute_u8,sunSetHour_u8,sunSetMinute_u8);
		Det_PrintLog(DET_MODULEID_DAYTIME, log);
#endif
		if ( (sunRiseHour_u8 > currentHour_u8) || ( (sunRiseHour_u8 == currentHour_u8) && (sunRiseMinute_u8 > currentMinute_u8) ))
		{
			/* Sunrise is after now() */
			retVal_en = DAYTIME_MOONLIGHT;
		}
		else
		{
			/* Sunrise is past */
			if ( (sunSetHour_u8 > currentHour_u8) || ((sunSetHour_u8 == currentHour_u8)  && (sunSetMinute_u8 > currentMinute_u8)))
			{
				/* Sunset is after now() */
				retVal_en = DAYTIME_SUNLIGHT;
			}
			else
			{
				/* Sunset is past */
				retVal_en = DAYTIME_MOONLIGHT;
			}
		}
	}
	return retVal_en;
}
#else

#include "DayTime.h"
#include "sunMoon.h"
#include "ntp.h"

DayTime_RetVal_ten DayTime_Get(void)
{
	DayTime_RetVal_ten retVal_en = DAYTIME_NOT_AVAILABLE;

	time_t currentTime;
	time_t sunRiseTime;
	time_t sunSetTime;

	currentTime = now();
	sunSetTime = SunMoon_SunSet(currentTime);
	sunRiseTime = SunMoon_SunRise(currentTime);

	if ( sunRiseTime > currentTime )
	{
		/* Sunrise is after now() */
		retVal_en = DAYTIME_MOONLIGHT;
	}
	else
	{
		/* Sunrise is past */
		if ((sunSetTime + 900u) > currentTime)
		{
			/* Sunset is after now() */
			retVal_en = DAYTIME_SUNLIGHT;
		}
		else
		{
			/* Sunset is past */
			retVal_en = DAYTIME_MOONLIGHT;
		}
	}
	return retVal_en;
}
#endif