/*
 * shCom.cpp
 *
 *  Created on: 2018. nov. 13.
 *      Author: tothpetiszilard
 */

#include "shCom.h"
#include "shCom_Cfg.h"
#ifdef ARDUINO
#include <WifiUdp.h>
#include <ESP8266WiFi.h>
#else
#include <esp_wifi.h>
#include "esp_system.h"
#include <string.h>
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "rom/ets_sys.h"
#endif

#if (10 < SH_SW_PATCH_VER)
#ifdef ARDUINO
#include "../Det/Det.h"
#else
#include "Det.h"
#endif
#endif

#ifdef ARDUINO
static WiFiUDP ucUdp;
#else
static int ucUdp;
typedef ip4_addr_t IPAddress;
#endif

static uint8_t shComBuffer_au8[SHCOM_UDP_MAXSIZE];
#ifndef ARDUINO
static void ShCom_Cyclic(void *pvParameters);
#endif
static inline void ShCom_SendResponse(IPAddress * srvIp,uint8_t * responseData_pu8, uint16_t len_u16);
static void HandleReceivedPacket(IPAddress serverIp, uint8_t * buffer_pu8, uint16_t size_u16);
#if (0 < PWM_CHS)
static void HandleReadPwm(IPAddress * srvIp, uint8_t chId_u8);
#endif
#if ( 0 < DIO_CHS )
static void HandleReadDio(IPAddress * srvIp, uint8_t chId_u8);
#endif
#if ( 0 < ADC_CHS )
static void HandleReadAdc(IPAddress * srvIp, uint8_t chId_u8);
#endif
#if ( 0 < TEMP_CHS )
static void HandleReadTemp(IPAddress * srvIp, uint8_t chId_u8);
static void HandleWriteTemp(IPAddress * srvIp, uint8_t chId_u8, uint8_t * data_pu8);
#endif

#if ( 0 < HUMI_CHS )
static void HandleReadHumi(IPAddress * srvIp, uint8_t chId_u8);
#endif

static void HandleReadState(IPAddress * srvIp);
static void HandleReadCfg(IPAddress * srvIp, uint8_t chId_u8);

#if (0 < PWM_CHS)
static void HandleWritePwm(IPAddress * srvIp, uint8_t chId_u8, uint16_t pwm_u16);
#endif

#if ((DEVSUBTYPE == 'W') && (DEVTYPE == 'A'))
static void HandleWriteWindow(uint8_t direction);
static void HandleReadWindow(IPAddress * srvIp);
#endif

static void HandleWriteCfg(IPAddress * srvIp, uint8_t chId_u8, uint8_t * cfg_pu8, uint16_t size_u16);
static void HandleWriteState(IPAddress * srvIp, uint8_t newState_u8);

void ShCom_Init(void)
{
#ifdef ARDUINO
	ucUdp.begin(SHCOM_UDP_PORT);
#else
	struct sockaddr_in srvAddr;
	ucUdp = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (0 > ucUdp)
	{
#if (10 < SH_SW_PATCH_VER)
			Det_ReportError(DET_MODULEID_SHCOM, "Unable to create UDP socket");
#endif
	}
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(SHCOM_UDP_PORT);
	srvAddr.sin_addr.s_addr = INADDR_ANY;
	bind(ucUdp, (struct sockaddr*)&srvAddr, sizeof(srvAddr));
	xTaskCreate(ShCom_Cyclic, "shCom", 2048, NULL, 5, NULL);
#endif
}

#ifndef ARDUINO
static void ShCom_Cyclic(void *pvParameters)
#else
void ShCom_Cyclic(void)
#endif
{
	uint16_t size_u16 = 0u;
#ifdef ARDUINO
	size_u16 = (uint16_t)ucUdp.available();
	if(SHCOM_UDP_MINSIZE < size_u16)
	{
		if (SHCOM_UDP_MAXSIZE < size_u16)
		{
			ucUdp.readBytes(shComBuffer_au8, SHCOM_UDP_MAXSIZE);
		}
		else
		{
			ucUdp.readBytes(shComBuffer_au8, size_u16);
		}
		HandleReceivedPacket(ucUdp.remoteIP(),shComBuffer_au8,size_u16);
		ucUdp.begin(SHCOM_UDP_PORT);
	}
#else
	while(1)
	{
		struct sockaddr_in remoteIp;
		IPAddress serverIP;
		socklen_t socklen = sizeof(remoteIp);
		size_u16 = recvfrom(ucUdp, shComBuffer_au8, SHCOM_UDP_MAXSIZE, 0, (struct sockaddr *)&remoteIp, &socklen);
		if (SHCOM_UDP_MINSIZE <= size_u16)
		{
			serverIP.addr = remoteIp.sin_addr.s_addr;
			HandleReceivedPacket(serverIP, shComBuffer_au8, size_u16);
		}
	}
#endif
}

static void HandleReceivedPacket(IPAddress serverIp, uint8_t * buffer_pu8, uint16_t size_u16)
{
	switch (buffer_pu8[1])
	{

	case 'C': /* Config*/
	case 'c': /* config*/
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/* Read */
			HandleReadCfg(&serverIp,buffer_pu8[2]);
		}
		else if (('W' == buffer_pu8[0]) || ('w' == buffer_pu8[0]))
		{
			/* Write */
			HandleWriteCfg(&serverIp,buffer_pu8[2], &buffer_pu8[3],size_u16);
		}
		break;

	case 'M': /* Mode*/
	case 'm': /* mode*/
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/* Read */
			HandleReadState(&serverIp);
		}
		else if (('W' == buffer_pu8[0]) || ('w' == buffer_pu8[0]))
		{
			/* Write */
			HandleWriteState(&serverIp,buffer_pu8[2]);
		}
		break;

	case 'P': /* PWM*/
	case 'p': /* PWM*/
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/* Read */
#if (0 < PWM_CHS)
			HandleReadPwm(&serverIp,buffer_pu8[2]);
#endif
		}
		else if (('W' == buffer_pu8[0]) || ('w' == buffer_pu8[0]))
		{
			/* Write */
#if (0 < PWM_CHS)
			HandleWritePwm(&serverIp, buffer_pu8[2], SHCOM_GETWORD(buffer_pu8[3],buffer_pu8[4]));
#endif
		}
		break;
	case 'd':
	case 'D' : /* DIO */
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/* Read */
#if (0 < DIO_CHS)
			HandleReadDio(&serverIp,buffer_pu8[2]);
#endif
		}
		else if (('W' == buffer_pu8[0]) || ('w' == buffer_pu8[0]))
		{
			/* Write */

		}
		break;
	case 'a':
	case 'A' : /* ADC */
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/* Read */
#if (0 < ADC_CHS)
			HandleReadAdc(&serverIp,buffer_pu8[2]);
#endif
		}

		break;
	case 'h':
	case 'H': /* Humidity */
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/*Read*/
#if (0 < HUMI_CHS)
			HandleReadHumi(&serverIp,buffer_pu8[2]);
#endif
		}
		break;
	case 't':
	case 'T' : /* Temperature */
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/* Read */
#if (0 < TEMP_CHS)
			HandleReadTemp(&serverIp,buffer_pu8[2]);
#endif
		}
		else if (('W' == buffer_pu8[0]) || ('w' == buffer_pu8[0]))
		{
			/* Write */
			//HandleWriteTemp(&serverIp,buffer_pu8[2],buffer_pu8);
		}
		break;
#if ((DEVSUBTYPE == 'L') && (DEVTYPE == 'D'))
	case 'g':
	case 'G' : /* Graphics */
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			/* Read */
		}
		else if (('W' == buffer_pu8[0]) || ('w' == buffer_pu8[0]))
		{
			/* Write */
			SHCOM_SETSTATE(SHSM_GFX);
			SHCOM_WRITEGFX(&buffer_pu8[2u],(size_u16-2u));
		}
		break;
#endif
#if ((DEVSUBTYPE == 'W') && (DEVTYPE == 'A'))
	case 'w':
	case 'W': /* Window */
		if (('W' == buffer_pu8[0]) || ('w' == buffer_pu8[0]))
		{
			HandleWriteWindow(buffer_pu8[2]);
		}
		if (('R' == buffer_pu8[0]) || ('r' == buffer_pu8[0]))
		{
			HandleReadWindow(&serverIp);
		}
		break;
#endif
	default:
		break;
	}
}

#if (0 < PWM_CHS)
static void HandleReadPwm(IPAddress * srvIp, uint8_t chId_u8)
{
	uint8_t responseData_au8[5];
	if (PWM_CHS <= chId_u8)
	{
		responseData_au8[0] = 'E'; /* Error */
		responseData_au8[1] = 'O'; /* Out */
		responseData_au8[2] = 'O'; /* Of */
		responseData_au8[3] = 'R'; /* Range */
	}
	else
	{
		responseData_au8[0] = 'D'; /* Data */
		responseData_au8[1] = 'P'; /* PWM */
		responseData_au8[2] = chId_u8; /* CH */
		SHCOM_GETPWM_PERCH(responseData_au8[2],&responseData_au8[3]); /* SetPoint */
	}
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}
#endif

#if (0 < DIO_CHS)
static void HandleReadDio(IPAddress * srvIp, uint8_t chId_u8)
{
	uint8_t responseData_au8[5];
	if (DIO_CHS <= chId_u8)
	{
		responseData_au8[0] = 'E'; /* Error */
		responseData_au8[1] = 'O'; /* Out */
		responseData_au8[2] = 'O'; /* Of */
		responseData_au8[3] = 'R'; /* Range */
	}
	else
	{
		responseData_au8[0] = 'D'; /* Data */
		responseData_au8[1] = 'D'; /* DIO */
		responseData_au8[2] = chId_u8; /* CH */
		SHCOM_GETDIO_PERCH(responseData_au8[2],&responseData_au8[3]); /* Value */
	}
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}
#endif

#if (0 < ADC_CHS)
static void HandleReadAdc(IPAddress * srvIp, uint8_t chId_u8)
{
	uint8_t responseData_au8[5];
	uint16_t adcVal_u16 = 0u;
	if (ADC_CHS <= chId_u8)
	{
		responseData_au8[0] = 'E'; /* Error */
		responseData_au8[1] = 'O'; /* Out */
		responseData_au8[2] = 'O'; /* Of */
		responseData_au8[3] = 'R'; /* Range */
	}
	else
	{
		responseData_au8[0] = 'D'; /* Data */
		responseData_au8[1] = 'A'; /* ADC */
		responseData_au8[2] = chId_u8; /* CH */
		adcVal_u16 = analogRead(A0);
		responseData_au8[3] = SHCOM_GETHIGHBYTE(adcVal_u16); /* Value */
		responseData_au8[4] = SHCOM_GETLOWBYTE(adcVal_u16);
	}
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}
#endif


static void HandleReadCfg(IPAddress * srvIp, uint8_t chId_u8)
{
	uint8_t cfgSize_u8 = SHCOM_GETSIZECFG(chId_u8);
	uint8_t responseData_au8[3u + cfgSize_u8];
	responseData_au8[0u] = 'D'; /* Data */
	responseData_au8[1u] = 'C'; /* Config */
	responseData_au8[2u] = chId_u8; /* CH */
	SHCOM_READCFG(chId_u8,&responseData_au8[3u]);
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}

#if (0 < HUMI_CHS)
static void HandleReadHumi(IPAddress * srvIp, uint8_t chId_u8)
{
	uint8_t responseData_au8[4];
		float humidity;
		if (HUMI_CHS <= chId_u8)
		{
			responseData_au8[0] = 'E'; /* Error */
			responseData_au8[1] = 'O'; /* Out */
			responseData_au8[2] = 'O'; /* Of */
			responseData_au8[3] = 'R'; /* Range */
		}
		else
		{
			humidity = SHCOM_GETHUMI();
			(void)chId_u8;
			responseData_au8[0] = 'D'; /* Data */
			responseData_au8[1] = 'H'; /* Humidity */
			responseData_au8[2] = chId_u8; /* CH */
			responseData_au8[3] = (uint8_t)humidity; /* Value */
		}
		ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}
#endif

#if (0 < TEMP_CHS)
static void HandleReadTemp(IPAddress * srvIp, uint8_t chId_u8)
{
	uint8_t responseData_au8[5];
	float temperature;
	if ((TEMP_CHS <= chId_u8) && (SHCOM_SETPOINT_CH != chId_u8))
	{
		responseData_au8[0] = 'E'; /* Error */
		responseData_au8[1] = 'O'; /* Out */
		responseData_au8[2] = 'O'; /* Of */
		responseData_au8[3] = 'R'; /* Range */
	}
	else
	{
		if (chId_u8 != SHCOM_SETPOINT_CH) temperature = SHCOM_GETTEMP();
		else temperature = SHCOM_GETTEMP_SP();

		responseData_au8[0] = 'D'; /* Data */
		responseData_au8[1] = 'T'; /* Temperature */
		responseData_au8[2] = chId_u8; /* CH */
		if (temperature < 0)
		{
			responseData_au8[3] = (uint8_t)temperature | 0x80u; /* Value */
		}
		else
		{
			responseData_au8[3] = (uint8_t)temperature; /* Value */
		}
		responseData_au8[4] = (uint8_t)(((float)temperature - (long)temperature) * (float)100); /* Value */
	}
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}
#endif

static void HandleReadState(IPAddress * srvIp)
{
	uint8_t responseData_au8[3];
	responseData_au8[0] = 'D'; /* Data */
	responseData_au8[1] = 'M'; /* Mode */
	responseData_au8[2] = SHCOM_GETSTATE();
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}

#if (0 < TEMP_CHS)
static void HandleWriteTemp(IPAddress * srvIp, uint8_t chId_u8, uint8_t * data_pu8)
{
	uint8_t responseData_au8[5];
	float temperature;
	if (SHCOM_SETPOINT_CH != chId_u8)
	{
		responseData_au8[0] = 'E'; /* Error */
		responseData_au8[1] = 'O'; /* Out */
		responseData_au8[2] = 'O'; /* Of */
		responseData_au8[3] = 'R'; /* Range */
	}
	else
	{
		if (0u != (data_pu8[3u] & 0x80u))
		{
			temperature = (float)(data_pu8[3u] & 0x7Fu) * -1.0f;

		}
		else
		{
			temperature = (float)(data_pu8[3u] & 0x7Fu);
		}
		temperature += (float)data_pu8[4u] / (100.0f);
		SHCOM_SETTEMP_SP(temperature);
		responseData_au8[0u] = 'A'; /* Data */
		responseData_au8[1u] = 'T'; /* Temperature */
		responseData_au8[2u] = chId_u8; /* CH */
		if (temperature < 0.0f)
		{
			responseData_au8[3] = (uint8_t)temperature | 0x80u; /* Value */
		}
		else
		{
			responseData_au8[3] = (uint8_t)temperature; /* Value */
		}
		responseData_au8[4] = (uint8_t)(((float)temperature - (long)temperature) * (float)100.0f); /* Value */
	}
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}
#endif

#if (0 < PWM_CHS)
static void HandleWritePwm(IPAddress * srvIp, uint8_t chId_u8, uint16_t pwm_u16)
{
	uint8_t responseData_au8[5];
	if ((PWM_CHS <= chId_u8) || (SHCOM_MAX_PWM_SETPOINT < pwm_u16))
	{
		responseData_au8[0] = 'E'; /* Error */
		responseData_au8[1] = 'O'; /* Out */
		responseData_au8[2] = 'O'; /* Of */
		responseData_au8[3] = 'R'; /* Range */
	}
	else if (SHCOM_PWMSET_ALLOWED_MODE_U8 != SHCOM_GETSTATE())
	{
		responseData_au8[0] = 'E'; /* Error */
		responseData_au8[1] = 'C'; /* Conditions */
		responseData_au8[2] = 'N'; /* Not */
		responseData_au8[3] = 'C'; /* Correct */
	}
	else
	{
		SHCOM_SETPWM_PERCH(chId_u8,pwm_u16);
		responseData_au8[0] = 'A'; /* Ack */
		responseData_au8[1] = 'P'; /* PWM */
		responseData_au8[2] = chId_u8; /* CH */
		responseData_au8[3] = SHCOM_GETHIGHBYTE(pwm_u16); /* Value */
		responseData_au8[4] = SHCOM_GETLOWBYTE(pwm_u16); /* Value */
	}
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}
#endif
static void HandleWriteCfg(IPAddress * srvIp, uint8_t chId_u8, uint8_t * cfg_pu8,uint16_t size_u16)
{
	uint8_t cfgSize_u8 = SHCOM_GETSIZECFG(chId_u8);
	if (cfgSize_u8 == (size_u16 - 3u))
	{
		SHCOM_WRITECFG(chId_u8,cfg_pu8);
		shComBuffer_au8[0u] = 'A'; /* Ack */
		shComBuffer_au8[1u] = 'C'; /* Config */
		shComBuffer_au8[2u] = chId_u8; /* CH */
		ShCom_SendResponse(srvIp,shComBuffer_au8, size_u16);
	}
	else
	{
		uint8_t responseData_au8[4u];
		responseData_au8[0u] = 'E'; /* Error */
		responseData_au8[1u] = 'O'; /* Out */
		responseData_au8[2u] = 'O'; /* Of */
		responseData_au8[3u] = 'R'; /* Range */
		ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
	}

}

static void HandleWriteState(IPAddress * srvIp, uint8_t newState_u8)
{
	uint8_t responseData_au8[3];
	responseData_au8[0] = 'A'; /* ACK */
	responseData_au8[1] = 'M'; /* Mode */
	if (newState_u8 != 255u)
	{
		SHCOM_SETSTATE(newState_u8);
	}
	else
	{
		esp_restart();
	}
	responseData_au8[2] = SHCOM_GETSTATE();
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}

#if ((DEVSUBTYPE == 'W') && (DEVTYPE == 'A'))

static void HandleReadWindow(IPAddress * srvIp)
{
	uint8_t responseData_au8[3];
	responseData_au8[0] = 'D'; /* Data */
	responseData_au8[1] = 'W'; /* Window */
	responseData_au8[2] = SHCOM_GET_WINDOWSTATE();
	ShCom_SendResponse(srvIp,responseData_au8,sizeof(responseData_au8));
}

static void HandleWriteWindow(uint8_t direction)
{
	if (1 == direction)
	{
		// Up
		SHCOM_WINDOW_UP();
	}
	else if (2 == direction)
	{
		// Down
		SHCOM_WINDOW_DOWN();
	}
	else if (3 == direction)
	{
		// Close
		SHCOM_WINDOW_CLOSE();
	}
	else
	{
		// STOP
		SHCOM_WINDOW_STOP();
	}
}
#endif

static inline void ShCom_SendResponse(IPAddress * srvIp,uint8_t * responseData_pu8, uint16_t len_u16)
{
#ifdef ARDUINO
	ucUdp.beginPacket(*srvIp, SHCOM_UDP_PORT);
	ucUdp.write(responseData_au8, len_u16);
	ucUdp.endPacket();
#else
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SHCOM_UDP_PORT);
	addr.sin_addr.s_addr = srvIp->addr;
	if (0 > sendto(ucUdp, responseData_pu8, len_u16, 0, (const struct sockaddr *) &addr, sizeof(addr)))
	{
#if (10 < SH_SW_PATCH_VER)
		Det_ReportError(DET_MODULEID_SHCOM, "UDP response send failed.\n");
#endif
	}
#endif
}
