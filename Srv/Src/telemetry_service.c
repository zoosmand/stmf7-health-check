/**
  ******************************************************************************
  * @file           : telemetry_service.c
  * @brief          : Runtime task and callback telemetry collection.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 28.09.2026
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

#include "telemetry_service.h"

#include "time_service.h"

#include <string.h>

static TaskHandle_t taskHandles[TELEMETRY_TASK_COUNT];

void TelemetryService_RegisterTask(
  TelemetryService_TaskTypeDef taskId,
  TaskHandle_t handle
) {
  if ((taskId >= TELEMETRY_TASK_COUNT) || (handle == NULL))
    return;
  /* Each slot has one initialization-time writer. Avoid entering a FreeRTOS
     critical section before the scheduler initializes port nesting state. */
  taskHandles[taskId] = handle;
}

void TelemetryService_GetSnapshot(
  TelemetryService_SnapshotTypeDef* snapshot
) {
  if (snapshot == NULL)
    return;
  memset(snapshot, 0, sizeof(*snapshot));
  snapshot->uptimeMs = TimeService_GetUptimeMs();
  TaskHandle_t handles[TELEMETRY_TASK_COUNT];
  taskENTER_CRITICAL();
  memcpy(handles, taskHandles, sizeof(handles));
  taskEXIT_CRITICAL();
  for (uint8_t index = 0U; index < TELEMETRY_TASK_COUNT; ++index) {
    if (handles[index] != NULL) {
      snapshot->stackMinFreeBytes[index] =
        (uint32_t)uxTaskGetStackHighWaterMark(handles[index])
        * (uint32_t)sizeof(StackType_t);
    }
  }
}
