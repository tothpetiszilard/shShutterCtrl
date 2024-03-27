#ifndef _SUN_MOON_H
#define _SUN_MOON_H

#include <time.h>
#include <ntp.h>
#include <math.h>
#include "Std_Types.h"

typedef enum
{
	day_dangerous, day_unhappy, day_normal, day_happy, day_happiest
}forecast_t;

StdReturnType SunMoon_Init(int Timezone, float Latitude, float Longitude);    // Timezone in minutes
uint32_t SunMoon_JulianDay(time_t date);
uint8_t SunMoon_MoonDay(time_t date);
time_t SunMoon_SunRise(time_t date);
time_t SunMoon_SunSet(time_t date);
forecast_t SunMoon_DayForecast(int8_t mDay );              // moonage [0-29]

#endif
