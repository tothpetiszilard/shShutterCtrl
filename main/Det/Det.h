/*
 * Det.h
 *
 *  Created on: Aug 31, 2020
 *      Author: tothpetiszilard
 */

#ifndef DET_H_
#define DET_H_

#include "Std_Types.h"
#include "Det_Cfg.h"

#ifdef ARDUINO
void Det_ReportError(uint8_t moduleId, const char * str);
void Det_PrintLog(uint8_t moduleId, const char * str);

#else
#include "esp_log.h"

#define Det_ReportError(x,y)	ESP_LOGE(x,y)
#define Det_PrintLog(x,y)		ESP_LOGI(x,y)

#endif

#endif /* DET_H_ */
