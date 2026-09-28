#ifndef CALLBACK_SERVICE_H
#define CALLBACK_SERVICE_H

#include "health_check_log.h"

#include <stdint.h>

/**
  * @brief Runtime-only callback delivery counters and last attempt result.
  * @param deliverySuccessCount (uint32_t) TLS-complete 2xx deliveries.
  * @param deliveryFailureCount (uint32_t) Transport or non-2xx failures.
  * @param droppedCount (uint32_t) Oldest queued events displaced on overflow.
  * @param lastElapsedMs (uint32_t) Last delivery attempt duration in ms.
  * @param lastDetail (int32_t) Last TlsTransport_ResultTypeDef detail code.
  * @param lastHttpStatus (uint16_t) Last response status, or zero.
  * @param lastTransportStatus (uint8_t) Last TlsTransport_StatusTypeDef value.
  * @param queueDepth (uint8_t) Pending failure events, from zero through three.
  */
typedef struct {
  uint32_t deliverySuccessCount;
  uint32_t deliveryFailureCount;
  uint32_t droppedCount;
  uint32_t lastElapsedMs;
  int32_t lastDetail;
  uint16_t lastHttpStatus;
  uint8_t lastTransportStatus;
  uint8_t queueDepth;
} CallbackService_TelemetryTypeDef;

/**
  * @brief Create the static callback queue and low-priority delivery task.
  * @retval (HealthCheck_StatusTypeDef) OK when both objects were created.
  */
HealthCheck_StatusTypeDef CallbackService_Init(void);

/**
  * @brief Queue one persisted failed check without blocking its producer.
  * @param entry (const HealthCheckLog_EntryTypeDef*) Non-null verified log
  *        entry for the failed check.
  * @note When the three-entry queue is full, the oldest event is discarded.
  */
void CallbackService_Enqueue(const HealthCheckLog_EntryTypeDef* entry);

/**
  * @brief Capture non-persistent callback delivery telemetry.
  * @param telemetry (CallbackService_TelemetryTypeDef*) Non-null output.
  * @note A delivery succeeds only when transport succeeds and HTTP is 2xx.
  */
void CallbackService_GetTelemetry(
  CallbackService_TelemetryTypeDef* telemetry
);

#endif /* CALLBACK_SERVICE_H */
