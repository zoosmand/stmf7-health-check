/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Application entry-point dependencies.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 29.09.2025
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

#ifndef MAIN_H
#define MAIN_H

#include "FreeRTOS.h"
#include "task.h"

#include "buzzer.h"
#include "buzzer_service.h"
#include "common.h"
#include "gpio.h"
#include "heart_beat.h"
#include "health_check_service.h"
#include "health_check_types.h"
#include "health_check_config.h"
#include "health_check_log.h"
#include "init_ll.h"
#include "network_service.h"
#include "spi.h"
#include "time_service.h"
#include "tls_platform.h"
#include "tls_trust_store.h"
#include "w25q64.h"
#include "watchdog_service.h"

#endif /* MAIN_H */
