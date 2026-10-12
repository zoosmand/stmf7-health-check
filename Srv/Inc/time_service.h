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

/**
  * @brief Create the SNTP synchronization task.
  * @retval (BaseType_t) pdPASS when the task was created; otherwise pdFAIL.
  */
BaseType_t TimeService_Init(void);
/** @retval (uint8_t) Nonzero after UTC has been synchronized. */
uint8_t TimeService_IsSynchronized(void);
/**
  * @brief Read synchronized UTC as Unix seconds.
  * @param unixTime (uint32_t*) Non-null output storage.
  * @retval (HealthCheck_StatusTypeDef) OK only while time is synchronized.
  */
HealthCheck_StatusTypeDef TimeService_GetUnixTime(uint32_t* unixTime);
/** @retval (uint32_t) Scheduler uptime in milliseconds, modulo 2^32. */
uint32_t TimeService_GetUptimeMs(void);
/**
  * @brief Apply a synchronized Unix timestamp to the RTC.
  * @param unixTime (uint32_t) UTC seconds since 1970-01-01.
  * @note Called by the lwIP SNTP notification in task context.
  */
void TimeService_SetUnixTime(uint32_t unixTime);

#endif /* TIME_SERVICE_H */
