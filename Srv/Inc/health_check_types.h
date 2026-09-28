/**
  ******************************************************************************
  * @file           : health_check_types.h
  * @brief          : Shared status types for health-check components.
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

#ifndef HEALTH_CHECK_TYPES_H
#define HEALTH_CHECK_TYPES_H

typedef enum {
  HEALTH_CHECK_STATUS_OK = 0,
  HEALTH_CHECK_STATUS_ERROR,
} HealthCheck_StatusTypeDef;

#endif /* HEALTH_CHECK_TYPES_H */
