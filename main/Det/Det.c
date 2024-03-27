/*
 * Det.cpp
 *
 *  Created on: Aug 31, 2020
 *      Author: tothpetiszilard
 */

#include "../shInfo.h"

#if (10 < SH_SW_PATCH_VER)

#include "Det.h"
#ifdef ARDUINO
#include "WifiUdp.h"
#include "../shCfg/shCfg.h"

#define DET_SHCFG_IPCH	(SHCFG_DETIP_CH)

static WiFiUDP ucUdp;

void Det_ReportError(uint8_t moduleId, const char * str)
{
	uint32_t ipAddress = 0;
	if (E_OK == ShCfg_Read(DET_SHCFG_IPCH, (uint8_t*)&ipAddress))
	{
		if (0 != ipAddress)
		{
			ucUdp.beginPacket(IPAddress(ipAddress), 60000);
			ucUdp.print("ERR:");
			ucUdp.print(moduleId,10);
			ucUdp.print(',');
			ucUdp.print(str);
			ucUdp.print('\n');
			ucUdp.endPacket();
		}
	}
}

void Det_PrintLog(uint8_t moduleId, const char * str)
{
	uint32_t ipAddress = 0;
	if (E_OK == ShCfg_Read(DET_SHCFG_IPCH, (uint8_t*)&ipAddress))
	{
		if (0 != ipAddress)
		{
			ucUdp.beginPacket(IPAddress(ipAddress), 60000);
			ucUdp.print("Info:");
			ucUdp.print(moduleId,10);
			ucUdp.print(',');
			ucUdp.print(str);
			ucUdp.print('\n');
			ucUdp.endPacket();
		}
	}
}

#endif //ARDUINO
#endif //SH_SW_PATCH_VER
