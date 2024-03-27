/*
 * tempSensor.h
 *
 *  Created on: 2018. nov. 26.
 *      Author: tothpetiszilard
 */

#ifndef TEMPSENSOR_H_
#define TEMPSENSOR_H_


extern void tempSensor_Init(void);
extern void tempSensor_Cyclic(void);
extern float tempSensor_GetTemp(void);
extern float tempSensor_GetHumidity(void);


#endif /* TEMPSENSOR_H_ */
