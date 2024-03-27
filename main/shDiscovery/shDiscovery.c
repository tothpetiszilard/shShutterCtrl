/*
 * shDiscovery.cpp
 *
 *  Created on: 2018. nov. 13.
 *      Author: tothpetiszilard
 */
#ifdef ARDUINO
#include <WifiUdp.h>
#include <ESP8266WiFi.h>
#else
#include <esp_wifi.h>
#include <string.h>
#include "lwip/igmp.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#endif
#include "shDiscovery.h"
#include "shDiscovery_Cfg.h"
#include "../shInfo.h"

#if (10 < SH_SW_PATCH_VER)
#include "../Det/Det.h"
#endif

#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
#define SHDISOVERY_SEARCH_DEVTYPE	    ('A') // Actuator
#define SHDISOVERY_SEARCH_DEVSUBTYPE	('P') // Power switch
#endif

typedef enum
{
	SHDISCOVERY_NODE,
	SHDISCOVERY_SEARCHER
} ShDiscoveryRoleType_ten;

#ifdef ARDUINO
static WiFiUDP mcastUdp;
static IPAddress myIP;
#else
 static int mcastUdp;
#endif
#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
static uint16_t pairedSN_u16;
#endif
static uint32_t delayTime_u32;
static uint8_t discoveryData_au8[SHDISCOVERY_PACKET_SIZE];
static uint8_t identData_au8 [] =
{
		DEVTYPE,
		DEVSUBTYPE,
		SERIAL_NUMBER_B0,
		SERIAL_NUMBER_B1,
		SH_SW_MAJOR_VER,
		SH_SW_MINOR_VER,
		SH_SW_PATCH_VER,
		PROTO_VER,
		PWM_CHS,
		ADC_CHS,
		DIO_CHS,
		TEMP_CHS,
		HUMI_CHS
};

static ShDiscoveryRoleType_ten role_en = SHDISCOVERY_NODE;

static inline void ShDiscovery_InitMcast(void);
#ifdef ARDUINO
#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
static inline void ShDiscovery_InitUcast(void);
#endif // SHDISCOVERY_SERACHER_MODE
#endif // ARDUINO
static inline void ShDiscovery_Receive(void);
#ifdef ARDUINO
static inline void ShDiscovery_HandleRx(IPAddress remoteIp);
#else
static void ShDiscovery_Cyclic(void *pvParameters);
static inline void ShDiscovery_HandleRx(ip4_addr_t * remoteIp);
#endif

void ShDiscovery_Init(void)
{
	uint8_t myMAC[6u];
#ifdef ARDUINO
	WiFi.macAddress(myMAC);
	myIP = WiFi.localIP();
#else
	esp_wifi_get_mac(ESP_IF_WIFI_STA, myMAC);
#endif
	ShCfg_Read(SHCFG_IDENTDATA_CH, identData_au8);

	delayTime_u32 = (uint32_t)myMAC[3u];
	delayTime_u32 |= ((uint32_t)myMAC[4u] << (uint32_t)8u);
	delayTime_u32 |= ((uint32_t)((uint8_t)myMAC[5u] & (uint8_t)0X0Fu) << (uint32_t)16u);
	role_en = SHDISCOVERY_NODE;
#ifdef ARDUINO
	ShDiscovery_InitMcast();
#else
	xTaskCreate(ShDiscovery_Cyclic, "shDiscovery", 2048, NULL, 5, NULL);
#endif
#if (10 < SH_SW_PATCH_VER)
	Det_PrintLog(DET_MODULEID_SHDISCOVERY, "Discovery init done");
#endif

}

#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
void ShDiscovery_OneSec(void)
{
	static uint8_t searchTimeout_u8 = 0u;
	if ((searchTimeout_u8 > 10u) && (SHDISCOVERY_SEARCHER == role_en))
	{
		/* Searching unsuccessfully for 10 seconds */
		role_en = SHDISCOVERY_NODE;
		ShDiscovery_InitMcast();
#if (10 < SH_SW_PATCH_VER)
		Det_ReportError(DET_MODULEID_SHDISCOVERY, "Node search failed");
#endif
	}
	else if (SHDISCOVERY_SEARCHER == role_en)
	{
		/* Searching unsuccessfully for less than 10 seconds */
		searchTimeout_u8++;
	}
	else
	{
		/* Not searching */
		searchTimeout_u8 = 0u;
	}
}
#endif

#ifndef ARDUINO
static void ShDiscovery_Cyclic(void *pvParameters)
{

	ShDiscovery_InitMcast(); // create socket
	while(1) // infinite loop in the task
	{
#else
void ShDiscovery_Cyclic(void)
{
#endif
	ShDiscovery_Receive();
#ifndef ARDUINO
	} // infinite loop in the task
#endif
}

static inline void ShDiscovery_Receive(void)
{
	#ifdef ARDUINO
	IPAddress srvIP;
	uint16_t rxSize_u16;
	rxSize_u16 = mcastUdp.available();
	if (4u <= rxSize_u16)
	{
		srvIP = mcastUdp.remoteIP();

		if (SHDISCOVERY_PACKET_SIZE < rxSize_u16)
		{
			mcastUdp.readBytes(discoveryData_au8, SHDISCOVERY_PACKET_SIZE);
		}
		else
		{
			mcastUdp.readBytes(discoveryData_au8, rxSize_u16);
		}

		ShDiscovery_HandleRx(srvIP);
	}
#else
    struct sockaddr_in remoteIp;
    socklen_t socklen = sizeof(remoteIp);
    int len = recvfrom(mcastUdp, discoveryData_au8, SHDISCOVERY_PACKET_SIZE, 0, (struct sockaddr *)&remoteIp, &socklen);
    if (4u <= len)
    {
    	ShDiscovery_HandleRx((ip4_addr_t *)&remoteIp.sin_addr);
    }
#endif
}

static inline void ShDiscovery_InitMcast(void)
{
#ifdef ARDUINO
	mcastUdp.beginMulticast(myIP, SHDISCOVERY_MCAST_IP, SHDISCOVERY_LOCALPORT_U16);
#else
	ip4_addr_t groupAddr;
	struct netif * netifPtr;
	struct sockaddr_in srvAddr;
	if (0 < ipaddr_aton(SHDISCOVERY_MCAST_IP,&groupAddr))
	{
		ESP_ERROR_CHECK(tcpip_adapter_get_netif(TCPIP_ADAPTER_IF_STA,(void **)&netifPtr));
		if (NULL != netifPtr)
		{
			igmp_joingroup_netif(netifPtr,&groupAddr);
		}
		mcastUdp = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
		if (0 > mcastUdp )
		{
#if (10 < SH_SW_PATCH_VER)
				Det_ReportError(DET_MODULEID_SHDISCOVERY, "Unable to create mcast socket");
#endif
		}
		srvAddr.sin_family = AF_INET;
		srvAddr.sin_port = htons(SHDISCOVERY_LOCALPORT_U16);
		srvAddr.sin_addr.s_addr = INADDR_ANY;
		bind(mcastUdp, (struct sockaddr*)&srvAddr, sizeof(srvAddr));
	}
#endif

}

#ifdef ARDUINO
#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
static inline void ShDiscovery_InitUcast(void)
{
	mcastUdp.begin(SHDISCOVERY_LOCALPORT_U16);
}
#endif // SHDISCOVERY_SERACHER_MODE
#endif // ARDUINO

#ifdef ARDUINO
static inline void ShDiscovery_HandleRx(IPAddress remoteIp)
#else
static inline void ShDiscovery_HandleRx(ip4_addr_t * remoteIp)
#endif
{
#ifdef ARDUINO
	WiFiUDP unicastUdp;
#else
	int unicastUdp;
#endif
#if (10 < SH_SW_PATCH_VER)
	char log[64];
#endif
#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
	uint16_t rxSerialNum_u16;
	if (SHDISCOVERY_NODE == role_en)
#endif
	{
		if (((discoveryData_au8[2u] == DEVTYPE) && (discoveryData_au8[3u] == DEVSUBTYPE))\
				|| ((discoveryData_au8[2u] == 255u) && (discoveryData_au8[3u] == 255u))\
				|| ((discoveryData_au8[2u] == DEVTYPE) && (discoveryData_au8[3u] == 255u)))
		{
	#if (10 < SH_SW_PATCH_VER)
			char log[64];
			#ifdef ARDUINO
			snprintf(log,63,"Rx discovery packet for %c %c from: %s",discoveryData_au8[2u],discoveryData_au8[3u],remoteIp.toString().c_str());
			#else
			snprintf(log,63,"Rx discovery packet for %c %c from: %i",discoveryData_au8[2u],discoveryData_au8[3u],remoteIp->addr);
			#endif
			Det_PrintLog(DET_MODULEID_SHDISCOVERY, log);
	#endif
			/* They are searching for us ! */
#ifdef ARDUINO
			delayMicroseconds(delayTime_u32); /* Delay a bit (max 1 sec) to avoid collisions */
			unicastUdp.beginPacket(remoteIp, SHDISCOVERY_LOCALPORT_U16);
			unicastUdp.write(identData_au8, sizeof(identData_au8));
			unicastUdp.endPacket();
#else
			ets_delay_us(delayTime_u32);
			unicastUdp = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(SHDISCOVERY_LOCALPORT_U16);
			addr.sin_addr.s_addr = remoteIp->addr;
			if (0 > sendto(unicastUdp, identData_au8, sizeof(identData_au8), 0, (const struct sockaddr *) &addr, sizeof(addr)))
			{
#if (10 < SH_SW_PATCH_VER)
				Det_ReportError(DET_MODULEID_SHDISCOVERY, "UDP response send failed.\n");
#endif
			}
			close(unicastUdp);
#endif
		}
#ifdef ARDUINO
		ShDiscovery_InitMcast();
#endif
	}
#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
	else
	{
		if ((discoveryData_au8[0u] == (uint8_t)SHDISOVERY_SEARCH_DEVTYPE) && (discoveryData_au8[1u] == (uint8_t)SHDISOVERY_SEARCH_DEVSUBTYPE))
		{
			rxSerialNum_u16 = ((uint16_t)discoveryData_au8[2u] << 8u);
			rxSerialNum_u16 |= discoveryData_au8[3u];
#if (10 < SH_SW_PATCH_VER)
			snprintf(log,63,"Rx NodeSN %d",rxSerialNum_u16);
			Det_PrintLog(DET_MODULEID_SHDISCOVERY, log);
#endif

			if (rxSerialNum_u16 == pairedSN_u16)
			{
#if (10 < SH_SW_PATCH_VER)
				#ifdef ARDUINO
				snprintf(log,63,"Found at %s",remoteIp.toString().c_str());
				#else
				snprintf(log,63,"Found at %d",remoteIp->addr);
				#endif
				Det_PrintLog(DET_MODULEID_SHDISCOVERY, log);
#endif
				SHDISCOVERY_RELAYIP_CB(remoteIp);
				role_en = SHDISCOVERY_NODE;
#ifdef ARDUINO
				ShDiscovery_InitMcast();
			}
			else
			{
				ShDiscovery_InitUcast();
#endif
			}
		}
	}
#endif
}

#if (STD_ON == (SHDISCOVERY_SERACHER_MODE))
void ShDiscovery_SendRequest(void)
{
#if (10 < SH_SW_PATCH_VER)
	char log[64];
#endif
	uint8_t discoData_au8[4u] =
	{
			DEVTYPE,DEVSUBTYPE,(uint8_t)SHDISOVERY_SEARCH_DEVTYPE,(uint8_t)SHDISOVERY_SEARCH_DEVSUBTYPE
	};
	uint8_t snData_u8[2u] = {0u,0u};

	SHDISCOVERY_GETPAIREDSN(snData_u8);
	pairedSN_u16 = (uint16_t)snData_u8[1u];
	pairedSN_u16 |= (uint16_t)((uint16_t)snData_u8[0u] << (uint16_t)8u);

#if (10 < SH_SW_PATCH_VER)
	snprintf(log,63,"Searching for %c %c with SN %d",SHDISOVERY_SEARCH_DEVTYPE,SHDISOVERY_SEARCH_DEVSUBTYPE,pairedSN_u16);
	Det_PrintLog(DET_MODULEID_SHDISCOVERY, log);
#endif

#ifdef ARDUINO
	mcastUdp.beginPacket(SHDISCOVERY_MCAST_IP, SHDISCOVERY_LOCALPORT_U16);
	mcastUdp.write(discoData_au8, sizeof(discoData_au8));
	mcastUdp.endPacket();
#else
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SHDISCOVERY_LOCALPORT_U16);
	addr.sin_addr.s_addr = inet_addr(SHDISCOVERY_MCAST_IP);
	if (0 > sendto(mcastUdp, discoData_au8, sizeof(discoData_au8), 0, (const struct sockaddr *) &addr, sizeof(addr)))
	{
#if (10 < SH_SW_PATCH_VER)
		Det_ReportError(DET_MODULEID_SHDISCOVERY, "UDP request send failed.\n");
#endif
	}
#endif
	role_en = SHDISCOVERY_SEARCHER;
#ifdef ARDUINO
	ShDiscovery_InitUcast();
#endif
}
#endif
