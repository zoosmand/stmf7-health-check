/**
  ******************************************************************************
  * @file           : network_service.h
  * @brief          : DHCP and fallback IPv4 network service interface.
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

#ifndef NETWORK_SERVICE_H
#define NETWORK_SERVICE_H

#include "FreeRTOS.h"

/**
  * @brief Create the statically allocated lwIP network-management task.
  * @retval (BaseType_t) pdPASS when created; otherwise pdFAIL.
  */
BaseType_t NetworkService_Init(void);

/**
  * @brief Check whether link and IPv4 configuration are both usable.
  * @retval 1 Link is up and DHCP or fallback parameters are assigned.
  * @retval 0 Link or address configuration is unavailable.
  */
uint8_t NetworkService_IsReady(void);

#endif /* NETWORK_SERVICE_H */
