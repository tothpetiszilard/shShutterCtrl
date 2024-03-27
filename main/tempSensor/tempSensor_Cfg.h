/*
 * tempSensor_Cfg.h
 *
 *  Created on: 2018. nov. 26.
 *      Author: tothpetiszilard
 */

#ifndef TEMPSENSOR_CFG_H_
#define TEMPSENSOR_CFG_H_

#include "../shCfg/shCfg.h"

#define TEMPSENSOR_DALLASPIN	(0u)
#define TEMPSENSOR_DHTPIN		(2u)
#define TEMPSENSOR_DHTTYPE		(DHTesp::DHT22)

#define TEMPSENSOR_TYPE_DHT		(1u)
#define TEMPSENSOR_TYPE_DALLAS	(2u)
#define TEMPSENSOR_TYPE_BOTH	(3u)

#define TEMPSENSOR_GETTYPE()	(ShCfg_Read(SHCFG_SENSORTYPE_CH,&config_u8))


#endif /* TEMPSENSOR_CFG_H_ */
