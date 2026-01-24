/*
 * shCom.h
 *
 *  Created on: 2018. nov. 13.
 *      Author: tothpetiszilard
 */

#ifndef SHCOM_H_
#define SHCOM_H_

#include "Std_Types.h"

extern void ShCom_Init(void);
#ifdef ARDUINO
extern void ShCom_Cyclic(void);
#endif

#endif /* SHCOM_H_ */
