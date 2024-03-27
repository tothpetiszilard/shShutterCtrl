/*
 * shDiscovery.h
 *
 *  Created on: 2018. nov. 13.
 *      Author: tothpetiszilard
 */

#ifndef SHDISCOVERY_H_
#define SHDISCOVERY_H_

#include "Std_Types.h"

extern void ShDiscovery_Init(void);
#ifdef ARDUINO
extern void ShDiscovery_Cyclic(void);
#endif
extern void ShDiscovery_OneSec(void);
extern void ShDiscovery_SendRequest(void);

#endif /* SHDISCOVERY_H_ */
