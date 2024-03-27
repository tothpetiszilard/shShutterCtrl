/*
 * MotorCtrl.h
 *
 *  Created on: May 7, 2020
 *      Author: tothpetiszilard
 */

#ifndef MOTORCTRL_H_
#define MOTORCTRL_H_

#include "stdint.h"

typedef enum
{
	MOTORCTRL_IDLE,
	MOTORCTRL_UP,
	MOTORCTRL_DOWN,
	MOTORCTRL_CLOSE,
}MotorCtrl_Status_ten;

extern void MotorCtrl_Init(void);
#ifdef ARDUINO
extern void MotorCtrl_Cyclic(void);
#endif
extern void MotorCtrl_Down(void);
extern void MotorCtrl_Stop(void);
extern void MotorCtrl_Up(void);
extern void MotorCtrl_Close(void);
extern uint8_t MotorCtrl_GetState(void);

extern void MotorCtrl_GetDioLevel(uint8_t chId_u8,uint8_t * val_pu8);

#endif /* MOTORCTRL_H_ */
