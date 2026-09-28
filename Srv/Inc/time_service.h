/**
  ******************************************************************************
  * @file           : time_service.h
  * @brief          : SNTP-backed UTC time service interface.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 02.08.2026
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2026 Dmitry Slobodchikov
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef TIME_SERVICE_H
#define TIME_SERVICE_H

#include "FreeRTOS.h"
#include "health_check_types.h"

#include <stdint.h>

BaseType_t TimeService_Init(void);
uint8_t TimeService_IsSynchronized(void);
HealthCheck_StatusTypeDef TimeService_GetUnixTime(uint32_t* unixTime);
uint32_t TimeService_GetUptimeMs(void);
void TimeService_SetUnixTime(uint32_t unixTime);

#endif /* TIME_SERVICE_H */
