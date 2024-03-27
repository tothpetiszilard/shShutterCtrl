/*
 * shCfg.cpp
 *
 *  Created on: Oct 29, 2019
 *      Author: tothpetiszilard
 */

#include "shCfg.h"
#include "shCfg_Cfg.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

StdReturnType ShCfg_Init(void)
{
	StdReturnType retVal = E_NOT_OK;
	uint8_t devID[4];
	esp_vfs_littlefs_conf_t fsCfg;
	fsCfg.base_path = "/littlefs";
	fsCfg.partition_label = "LittleFS";
	fsCfg.format_if_mount_failed = 1u;
	fsCfg.dont_mount = 0u;
	if (ESP_OK != esp_vfs_littlefs_register(&fsCfg))
	{
		esp_littlefs_format("LittleFS");
/*		devID[0] = (uint8_t)('D');
		devID[1] = (uint8_t)('L');
		devID[2] = (uint8_t)(254);
		devID[3] = (uint8_t)(254);
		ShCfg_Write(SHCFG_IDENTDATA_CH, devID);*/
		retVal = E_OK;
	}
	else
	{
		retVal = ShCfg_Read(SHCFG_IDENTDATA_CH, devID);
		if (E_OK != retVal)
		{
			devID[0] = (uint8_t)('A');
			devID[1] = (uint8_t)('W');
			devID[2] = (uint8_t)(0);
			devID[3] = (uint8_t)(24);
			ShCfg_Write(SHCFG_IDENTDATA_CH, devID);
			uint8_t ssid[] = "shNetwork\0";
			uint8_t pwd[] = "d0ntH4ck1t\0";
			if (E_OK == ShCfg_Write(0u,ssid))
			{
				retVal = ShCfg_Write(1u,pwd);
			}
		}
	}
	return retVal;
}
/*
String ShCfg_GetString(uint8_t chId_u8)
{
	String retVal_s = "";
	uint8_t tmp_u8 = 0u;
	String filename = "/";
	for (tmp_u8 = 0u; tmp_u8 < SHCFG_CONFIGSIZE; tmp_u8++)
	{
		if (chId_u8 == shCfg_ConfigTable_cast[tmp_u8].chId_cu8)
		{
			filename += String(chId_u8);
			filename += ".cfg";
			File cfg = LittleFS.open(filename, "r");
			if(shCfg_ConfigTable_cast[tmp_u8].size_cu8 <= (cfg.size()))
			{
				retVal_s = cfg.readString();
			}
			cfg.close();
			break;
		}
	}
	return retVal_s;
}

const char * ShCfg_GetCharArray(uint8_t chId_u8)
{
	String retVal_s = "";
	uint8_t tmp_u8 = 0u;
	String filename = "/";
	for (tmp_u8 = 0u; tmp_u8 < SHCFG_CONFIGSIZE; tmp_u8++)
	{
		if (chId_u8 == shCfg_ConfigTable_cast[tmp_u8].chId_cu8)
		{
			filename += String(chId_u8);
			filename += ".cfg";
			File cfg = LittleFS.open(filename, "r");
			if(shCfg_ConfigTable_cast[tmp_u8].size_cu8 <= (cfg.size()))
			{
				retVal_s = cfg.readString();
			}
			cfg.close();
			break;
		}
	}
	return retVal_s.c_str();
}
*/
uint8_t ShCfg_GetSize(uint8_t chId_u8)
{
	uint8_t retVal_u8 = 0u;
	uint8_t tmp_u8 = 0u;
	for (tmp_u8 = 0u; tmp_u8 < SHCFG_CONFIGSIZE; tmp_u8++)
	{
		if (chId_u8 == shCfg_ConfigTable_cast[tmp_u8].chId_cu8)
		{
			retVal_u8 = shCfg_ConfigTable_cast[tmp_u8].size_cu8;
			break;
		}
	}
	return retVal_u8;
}

StdReturnType ShCfg_Read(uint8_t ch_u8, uint8_t * data_pu8)
{
	StdReturnType retVal_u8 = E_NOT_OK;
	int result = -1;
	uint8_t tmp_u8 = 0u;
	char filename[32] = "";
	struct stat sb;
	for (tmp_u8 = 0u; tmp_u8 < SHCFG_CONFIGSIZE; tmp_u8++)
	{
		if (ch_u8 == shCfg_ConfigTable_cast[tmp_u8].chId_cu8)
		{
			snprintf(filename,32,"/littlefs/%d.cfg",ch_u8);
			FILE * cfg = fopen(filename, "r");
			if ((cfg != NULL) && (0 == stat(filename,&sb)))
			{
				if(shCfg_ConfigTable_cast[tmp_u8].size_cu8 <= ( sb.st_size))
				{
					result = fread(data_pu8,1, shCfg_ConfigTable_cast[tmp_u8].size_cu8,cfg);
					if (0 < result)
					{
						retVal_u8 = E_OK;
					}
				}
			}
			fclose(cfg);
			break;
		}
	}
	return retVal_u8;
}

StdReturnType ShCfg_Write(uint8_t ch_u8, uint8_t * data_pu8)
{
	uint8_t tmp_u8 = 0u;
	StdReturnType retVal_u8 = E_NOT_OK;
	int result = -1;
	char filename[32] = "";
	for (tmp_u8 = 0u; tmp_u8 < SHCFG_CONFIGSIZE; tmp_u8++)
	{
		if (ch_u8 == shCfg_ConfigTable_cast[tmp_u8].chId_cu8)
		{
			snprintf(filename,32,"/littlefs/%d.cfg",ch_u8);
			FILE * cfg = fopen(filename, "w");
			if (NULL != cfg)
			{
				result = fwrite(data_pu8, 1,shCfg_ConfigTable_cast[tmp_u8].size_cu8,cfg);
				if (0 < result)
				{
					retVal_u8 = E_OK;
				}
				fclose(cfg);
			}

			break;
		}
	}
	return retVal_u8;
}
