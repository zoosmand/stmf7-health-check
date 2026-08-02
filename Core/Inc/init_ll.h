/**
  ******************************************************************************
  * @file           : init_ll.h
  * @brief          : Board-level peripheral initialization interface.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 01.10.2025
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

#ifndef INIT_LL_H
#define INIT_LL_H

#include "ethernet_ll.h"

/**
  * @brief Configure RMII pins and initialize the Ethernet MAC and LAN8742 PHY.
  * @retval (Ethernet_StatusTypeDef) Initialization result.
  */
Ethernet_StatusTypeDef Board_InitEthernet(void);

#endif /* INIT_LL_H */
