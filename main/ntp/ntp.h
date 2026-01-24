/*
 * ntp.h
 *
 *  Created on: Dec 12, 2020
 *      Author: tothpetiszilard
 */

#ifndef NTP_H_
#define NTP_H_

#include <time.h>

typedef enum
{
    dowSunday = 0u,
    dowMonday = 1u,
    dowTuesday,
    dowWednesday,
    dowThursday,
    dowFriday,
    dowSaturday,
    dowInvalid
} timeDayOfWeek_t;

void NTP_Init(void);

uint32_t NTP_getUTCOffset(void);

uint8_t hour(time_t t);
uint8_t minute(time_t t);
uint8_t second(time_t t);
uint16_t year(time_t t);
uint8_t month(time_t t);
uint8_t day(time_t t);
timeDayOfWeek_t weekday(time_t t);

time_t now(void);

#endif /* NTP_H_ */
