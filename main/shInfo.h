/*
 * shInfo.h
 *
 *  Created on: 2018. nov. 13.
 *      Author: tothpetiszilard
 */

#ifndef SHINFO_H_
#define SHINFO_H_

#ifdef ARDUINO
#include "Arduino.h"
#else
#include <stdint.h>
#endif

#define xstr(s) str(s)
#define str(s) #s

#define TYPE					A
#define SUBTYPE					W
#define VER_MAJOR				1
#define VER_MINOR				6
#define VER_PATCH				6

#define DEVTYPE					('A')
#define DEVSUBTYPE				('W')
#define PWM_CHS					(0u)
#define ADC_CHS					(0u)
#define DIO_CHS					(0u)
#define TEMP_CHS				(0u)
#define HUMI_CHS				(0u)
#define SERIAL_NUMBER_B0		((uint8_t)(0x00u))
#define SERIAL_NUMBER_B1		((uint8_t)(0x0Eu))
#define SH_SW_MAJOR_VER			((uint8_t)(VER_MAJOR))
#define SH_SW_MINOR_VER			((uint8_t)(VER_MINOR))
#define SH_SW_PATCH_VER			VER_PATCH
#define PROTO_VER				((uint8_t)(0x05u))

#endif /* SHINFO_H_ */
