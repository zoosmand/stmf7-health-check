/**
  ******************************************************************************
  * @file           : time_service.c
  * @brief          : SNTP-backed UTC time service.
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

#include "time_service.h"

#include "health_check_types.h"
#include "common.h"
#include "network_service.h"

#include "lwip/apps/sntp.h"
#include "lwip/tcpip.h"
#include "task.h"

#include <stdio.h>

#define TIME_TASK_STACK_WORDS 384U
#define TIME_TASK_PRIORITY    (tskIDLE_PRIORITY + 1U)
#define TIME_WAIT_MS          1000U
#define TIME_SNTP_SERVER      "pool.ntp.org"

static StaticTask_t timeTaskControlBlock;
static StackType_t timeTaskStack[TIME_TASK_STACK_WORDS];
static volatile uint32_t synchronizedUnixTime;
static volatile TickType_t synchronizedAtTick;
static volatile uint8_t synchronized;

static void timeService_Task(void* argument);
static void timeService_StartSntp(void* argument);

BaseType_t TimeService_Init(void) {
  TaskHandle_t task = xTaskCreateStatic(
    timeService_Task,
    "time",
    TIME_TASK_STACK_WORDS,
    NULL,
    TIME_TASK_PRIORITY,
    timeTaskStack,
    &timeTaskControlBlock
  );
  return task != NULL ? pdPASS : pdFAIL;
}

uint8_t TimeService_IsSynchronized(void) {
  return synchronized;
}

HealthCheck_StatusTypeDef TimeService_GetUnixTime(uint32_t* unixTime) {
  if ((unixTime == NULL) || (synchronized == 0U))
    return HEALTH_CHECK_STATUS_ERROR;
  taskENTER_CRITICAL();
  uint32_t base = synchronizedUnixTime;
  TickType_t baseTick = synchronizedAtTick;
  taskEXIT_CRITICAL();
  *unixTime = base
    + ((xTaskGetTickCount() - baseTick) / configTICK_RATE_HZ);
  return HEALTH_CHECK_STATUS_OK;
}

uint32_t TimeService_GetUptimeMs(void) {
  return (uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS);
}

void TimeService_SetUnixTime(uint32_t unixTime) {
  taskENTER_CRITICAL();
  synchronizedUnixTime = unixTime;
  synchronizedAtTick = xTaskGetTickCount();
  synchronized = 1U;
  taskEXIT_CRITICAL();
}

static void timeService_Task(void* argument) {
  (void)argument;
  while (NetworkService_IsReady() == 0U)
    vTaskDelay(pdMS_TO_TICKS(TIME_WAIT_MS));
  Common_Printf("NTP: synchronizing with %s.\n", TIME_SNTP_SERVER);
  if (tcpip_callback_with_block(timeService_StartSntp, NULL, 1U) != ERR_OK)
    System_ErrorHandler();
  uint8_t reported = 0U;
  for (;;) {
    if ((synchronized != 0U) && (reported == 0U)) {
      Common_Printf("NTP: synchronized with %s.\n", TIME_SNTP_SERVER);
      reported = 1U;
    }
    vTaskDelay(pdMS_TO_TICKS(TIME_WAIT_MS));
  }
}

static void timeService_StartSntp(void* argument) {
  (void)argument;
  sntp_setoperatingmode(SNTP_OPMODE_POLL);
  sntp_setservername(0U, TIME_SNTP_SERVER);
  sntp_init();
}
