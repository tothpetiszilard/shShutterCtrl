/*
 * tempSensor.cpp
 *
 *  Created on: 2018. nov. 26.
 *      Author: tothpetiszilard
 */

#include "tempSensor.h"
#include "tempSensor_Cfg.h"
#include "DHTesp.h"
#include <OneWire.h>
#include <DallasTemperature.h>

static DHTesp dht;
static OneWire ow(TEMPSENSOR_DALLASPIN);
static DallasTemperature ds(&ow);
static float dhtHumidity = 0.0f;
static float dsTemp = 0.0f;
static float dhtTemp = 0.0f;

static uint8_t config_u8 = 0u; /* 0 = INIT , 1 = DHT , 2 = DALLAS, 3 = Both */

void tempSensor_Init(void)
{
	if (E_OK == TEMPSENSOR_GETTYPE())
	{
		if (TEMPSENSOR_TYPE_DHT == config_u8)
		{
			/* DHT22 installed */
			pinMode(TEMPSENSOR_DHTPIN, INPUT);
			delay(1000ul);
			dht.setup(TEMPSENSOR_DHTPIN, TEMPSENSOR_DHTTYPE);
		}
		else if (TEMPSENSOR_TYPE_DALLAS == config_u8)
		{
			/* DS18B20 installed */
			ds.begin();
			ds.setResolution(9u);
			delay(1000ul);
			ds.requestTemperatures();
		}
		else if (TEMPSENSOR_TYPE_BOTH == config_u8)
		{
			/* DHT22 installed */
			pinMode(TEMPSENSOR_DHTPIN, INPUT);
			delay(1000ul);
			dht.setup(TEMPSENSOR_DHTPIN, TEMPSENSOR_DHTTYPE);
			/* DS18B20 installed */
			ds.begin();
			ds.setResolution(9u);
			delay(1000ul);
			ds.requestTemperatures();
		}
		else
		{
			/* Invalid type */
		}
	}

}

void tempSensor_Cyclic(void)
{
	uint8_t cfgData = 0;
	TempAndHumidity data;
	if (E_OK == ShCfg_Read(SHCFG_SENSORTYPE_CH, &cfgData))
	{
		if (cfgData != config_u8)
		{
			//tempSensor_Init(); // Config changed, reinit
			ESP.reset();
		}
		else
		{
			if (TEMPSENSOR_TYPE_DHT == config_u8)
			{
				data = dht.getTempAndHumidity();
				dhtHumidity = data.humidity;
				dhtTemp = data.temperature;
			}
			else if (TEMPSENSOR_TYPE_DALLAS == config_u8)
			{
				dsTemp = ds.getTempCByIndex(0u);
				ds.requestTemperatures();
			}
			else if (TEMPSENSOR_TYPE_BOTH == config_u8)
			{
				data = dht.getTempAndHumidity();
				if (88.8f != data.temperature)
				{
					dhtHumidity = data.humidity;
					dhtTemp = data.temperature;
				}
				else
				{
					dhtTemp = 0.0f;
					dhtHumidity = 0.0f;
				}
				dsTemp = ds.getTempCByIndex(0u);
				ds.requestTemperatures();
			}
			else
			{
				/* No sensor */
			}
		}
	}

}

float tempSensor_GetTemp(void)
{
	float retVal_f = 0.0f;
	if (TEMPSENSOR_TYPE_DHT == config_u8)
	{
		retVal_f= dhtTemp;
	}
	else if (TEMPSENSOR_TYPE_DALLAS == config_u8)
	{
		retVal_f= dsTemp;
	}
	else if ((TEMPSENSOR_TYPE_BOTH == config_u8) && (0.0f != dhtTemp))
	{
		retVal_f= ((( 2.0 * dsTemp) + dhtTemp) / 3.0f);
	}
	else if ((TEMPSENSOR_TYPE_BOTH == config_u8) && (0.0f == dhtTemp))
	{
		retVal_f= dsTemp;
	}
	else
	{
		/* No sensor */
	}
	return retVal_f;
}

float tempSensor_GetHumidity(void)
{
	return dhtHumidity;
}
