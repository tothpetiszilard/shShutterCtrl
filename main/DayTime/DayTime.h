/*
 * SunRise.h
 *
 *  Created on: 2018. nov. 10.
 *      Author: tothpetiszilard
 */

#ifndef DAYTIME_H_
#define DAYTIME_H_

#ifdef ARDUINO
#include "Arduino.h"
#endif

typedef enum _DayTime_RetVal_ten
{
	DAYTIME_NOT_AVAILABLE,
	DAYTIME_SUNLIGHT,
	DAYTIME_MOONLIGHT
}DayTime_RetVal_ten;

#ifdef ARDUINO
extern void DayTime_Cyclic(void);
extern void DayTime_Update(void);
#endif
extern DayTime_RetVal_ten DayTime_Get(void);

#endif /* DAYTIME_H_ */
