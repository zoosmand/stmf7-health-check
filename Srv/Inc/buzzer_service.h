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
  * @brief Schedule the configured alert pattern for a failed resource check.
  * @note Non-blocking. A pattern already queued or playing absorbs the
  *       request instead of queuing a second one, so repeated failures never
  *       grow an unbounded backlog. Safe to call from any task.
  */
void BuzzerService_Alert(void);

/**
  * @brief Schedule the distinct factory-reset acknowledgement pattern.
  * @note This event replaces any queued resource-alert event.
  */
void BuzzerService_FactoryResetSignal(void);

#endif /* BUZZER_SERVICE_H */
