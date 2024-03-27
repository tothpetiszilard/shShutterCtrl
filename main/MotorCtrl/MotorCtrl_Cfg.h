
#ifdef ARDUINO

#include "Arduino.h"
#include "TimeLib.h"
#include "../shCfg/shCfg.h"

#define MOTORCTRL_PIN_UP      (0u)
#define MOTORCTRL_PIN_DOWN    (2u)

#define MOTORCTRL_ON          (LOW)
#define MOTORCTRL_OFF         (HIGH)

#else

#include <sys/time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "esp_task_wdt.h"
#include "shCfg.h"

#define digitalWrite(pin,val)	(gpio_set_level(pin,val))
#define millis()                (xTaskGetTickCount())

#define MOTORCTRL_PIN_UP      (GPIO_NUM_5)
#define MOTORCTRL_PIN_DOWN    (GPIO_NUM_14)
#define MOTORCTRL_PIN_EN      (GPIO_NUM_4) // Enable

#define MOTORCTRL_ON          (1u)
#define MOTORCTRL_OFF         (0u)

#endif

#define MOTORCTRL_DRIVETIME_UP     (1000UL) // 1 seconds more than actual pos
#define MOTORCTRL_DRIVETIME_DOWN   (12530UL) // 12.53 seconds
#define MOTORCTRL_DRIVETIME_CLOSE  (17000UL) // 17 seconds

#define MOTORCTRL_MOTORPOS_UP	   (0u)
#define MOTORCTRL_MOTORPOS_DOWN	   (MOTORCTRL_DRIVETIME_DOWN)
#define MOTORCTRL_MOTORPOS_CLOSE   (MOTORCTRL_DRIVETIME_CLOSE)
