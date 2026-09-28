/**
  ******************************************************************************
  * @file           : factory_reset_service.h
  * @brief          : Power-loss-safe factory-reset service interface.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 02.09.2026
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

#ifndef FACTORY_RESET_SERVICE_H
#define FACTORY_RESET_SERVICE_H

#include "FreeRTOS.h"

#include "w25q64.h"

/**
  * @brief Complete an interrupted factory reset before persistent stores load.
  * @retval (W25Q64_StatusTypeDef) Marker read or reset completion status.
  * @note A completed pending reset restarts the MCU and does not return.
  */
W25Q64_StatusTypeDef FactoryResetService_ResumePending(void);

/**
  * @brief Create the B1 long-press monitoring task.
  * @retval (BaseType_t) pdPASS when the task was created; otherwise pdFAIL.
  */
BaseType_t FactoryResetService_Init(void);

#endif /* FACTORY_RESET_SERVICE_H */
