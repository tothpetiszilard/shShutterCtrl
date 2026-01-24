/*
 * Std_Types.h
 *
 *  Created on: Jan 9, 2020
 *      Author: tothpetiszilard
 */

#ifndef STD_TYPES_H_
#define STD_TYPES_H_

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdint.h>
#endif

typedef uint8_t 	StdReturnType;

#define E_OK		((uint8_t)0u)
#define E_NOT_OK	((uint8_t)1u)
#define E_PENDING	((uint8_t)2u)

#define STD_OFF		(0u)
#define STD_ON		(1u)

#endif /* STD_TYPES_H_ */
