/**
  ******************************************************************************
  * @file           : heart_beat.h
  * @brief          : Heartbeat service interface.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 05.10.2025
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

#ifndef HEART_BEAT_H
#define HEART_BEAT_H

#include "FreeRTOS.h"

/**
  * @brief Create the statically allocated heartbeat task.
  * @retval (BaseType_t) pdPASS when the task was created, otherwise pdFAIL.
  */
BaseType_t HeartBeatService_Init(void);

#endif /* HEART_BEAT_H */
