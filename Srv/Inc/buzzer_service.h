/**
  ******************************************************************************
  * @file           : buzzer_service.h
  * @brief          : FreeRTOS-aware buzzer alert service interface.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 03.08.2026
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

#ifndef BUZZER_SERVICE_H
#define BUZZER_SERVICE_H

#include "FreeRTOS.h"

/**
  * @brief Create the statically allocated buzzer task and play the startup
  *        self-test tone once the task starts running.
  * @retval (BaseType_t) pdPASS when the task was created, otherwise pdFAIL.
  * @note Call Buzzer_Init() before this function.
  */
BaseType_t BuzzerService_Init(void);

/**
  * @brief Schedule the failed-health-check warning and resource ID beeps.
  * @param resourceId (uint8_t) One-based resource ID and following beep count.
  * @retval (BaseType_t) pdPASS when queued, otherwise pdFAIL.
  */
BaseType_t BuzzerService_HealthCheckAlert(uint8_t resourceId);

/**
  * @brief Schedule the certificate-expiry melody and resource ID beeps.
  * @param resourceId (uint8_t) One-based resource ID and following beep count.
  * @retval (BaseType_t) pdPASS when queued, otherwise pdFAIL.
  */
BaseType_t BuzzerService_CertificateWarning(uint8_t resourceId);

/**
  * @brief Schedule five warning beeps before the reset cancellation window.
  * @note This event replaces any queued resource-alert event.
  */
void BuzzerService_FactoryResetWarning(void);

/**
  * @brief Return the scheduled five-beep warning pattern duration.
  * @retval (TickType_t) Pattern duration in RTOS ticks.
  * @note Use this value instead of duplicating the buzzer timing constants in
  *       callers that must wait for the warning pattern to finish.
  */
TickType_t BuzzerService_FactoryResetWarningDuration(void);

/**
  * @brief Schedule three beeps acknowledging factory-reset cancellation.
  * @note This event replaces any queued resource-alert event.
  */
void BuzzerService_FactoryResetCancelled(void);

/**
  * @brief Schedule the distinct factory-reset failure pattern.
  * @note This event replaces any queued resource-alert event.
  */
void BuzzerService_FactoryResetFailure(void);

#endif /* BUZZER_SERVICE_H */
