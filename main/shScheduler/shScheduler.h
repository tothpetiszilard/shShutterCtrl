/*
 * shScheduler.h
 *
 *  Created on: Jan 9, 2020
 *      Author: tothpetiszilard
 */

#ifndef SHSCHEDULER_H_
#define SHSCHEDULER_H_

#ifdef ARDUNINO
#include "../Std_Types.h"
#else
#include "Std_Types.h" 
#endif

extern void ShScheduler_Init(void);
#ifdef ARDUNINO
extern void ShScheduler_Cyclic(void);
#endif

#endif /* SHSCHEDULER_H_ */
