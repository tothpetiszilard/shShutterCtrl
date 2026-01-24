/*
 * shStateMachine_Cfg.h
 *
 *  Created on: 2018. nov. 14.
 *      Author: tothpetiszilard
 */

#ifndef SHSTATEMACHINE_CFG_H_
#define SHSTATEMACHINE_CFG_H_


#ifdef ARDUINO
#include "TimeLib.h"
#include "../shCfg/shCfg.h"
#else
#include <time.h>
#include "shCfg.h"
#endif
#include <Std_Types.h>

#ifdef ARDUINO
#define SHSM_GETTIME()				(now())
#else
#define SHSM_GETTIME()				(time(NULL))
#endif

#define SHSM_GFXTIME				((time_t)(1u)) // 1 sec


#endif /* SHSTATEMACHINE_CFG_H_ */
