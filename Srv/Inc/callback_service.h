#ifndef CALLBACK_SERVICE_H
#define CALLBACK_SERVICE_H

#include "health_check_log.h"

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

#endif /* CALLBACK_SERVICE_H */
