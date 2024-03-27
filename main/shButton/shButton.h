/*
 * shButton.h
 *
 *  Created on: Sep 20, 2020
 *      Author: tothpetiszilard
 */

#ifndef SHBUTTON_H_
#define SHBUTTON_H_

#ifdef ARDUINO
#include "../Std_Types.h"
#else
#include "Std_Types.h"
#endif

void ShButton_Read(uint8_t ch_u8, uint8_t * val_pu8);

void ShButton_Init(void);
#ifdef ARDUINO
void ShButton_Cyclic(void);
#endif

#endif /* SHBUTTON_H_ */
