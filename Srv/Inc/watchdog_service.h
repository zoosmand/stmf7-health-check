/**
  ******************************************************************************
  * @file           : watchdog_service.h
  * @brief          : Independent watchdog (IWDG) supervision service interface.
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

#ifndef WATCHDOG_SERVICE_H
#define WATCHDOG_SERVICE_H

#include "FreeRTOS.h"

/**
  * @brief Create the statically allocated watchdog task.
  *
  * Once the scheduler is running, the task starts the IWDG (nominally a
  * 2.4 s timeout) and reloads it on a period well inside that window. If the
  * scheduler ever stops making forward progress on this task, the IWDG times
  * out and resets the system.
  * @retval (BaseType_t) pdPASS when the task was created, otherwise pdFAIL.
  */
BaseType_t WatchdogService_Init(void);

#endif /* WATCHDOG_SERVICE_H */
