#ifndef TELEMETRY_SERVICE_H
#define TELEMETRY_SERVICE_H

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

typedef enum {
  TELEMETRY_TASK_HEARTBEAT = 0,
  TELEMETRY_TASK_WATCHDOG,
  TELEMETRY_TASK_FACTORY_RESET,
  TELEMETRY_TASK_NETWORK,
  TELEMETRY_TASK_TIME,
  TELEMETRY_TASK_BUZZER,
  TELEMETRY_TASK_HEALTH_CHECK,
  TELEMETRY_TASK_CALLBACK,
  TELEMETRY_TASK_API,
  TELEMETRY_TASK_COUNT
} TelemetryService_TaskTypeDef;

/**
  * @brief One bounded runtime telemetry snapshot.
  * @param uptimeMs (uint32_t) Milliseconds since scheduler start; wraps.
  * @param stackMinFreeBytes (uint32_t[]) Minimum-ever unused stack bytes per
  *        registered TelemetryService_TaskTypeDef slot.
  */
typedef struct {
  uint32_t uptimeMs;
  uint32_t stackMinFreeBytes[TELEMETRY_TASK_COUNT];
} TelemetryService_SnapshotTypeDef;

/**
  * @brief Register one statically allocated application task for telemetry.
  * @param taskId (TelemetryService_TaskTypeDef) Stable telemetry slot.
  * @param handle (TaskHandle_t) Non-null handle returned by xTaskCreateStatic.
  * @note May be called before or after the scheduler starts.
  */
void TelemetryService_RegisterTask(
  TelemetryService_TaskTypeDef taskId,
  TaskHandle_t handle
);

/**
  * @brief Capture uptime and minimum-ever unused task stack space.
  * @param snapshot (TelemetryService_SnapshotTypeDef*) Non-null output.
  * @note Stack values are bytes; zero means the task was not registered.
  */
void TelemetryService_GetSnapshot(
  TelemetryService_SnapshotTypeDef* snapshot
);

#endif /* TELEMETRY_SERVICE_H */
