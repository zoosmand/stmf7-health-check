/**
  ******************************************************************************
  * @file           : ethernet_ll.h
  * @brief          : STM32F767 Ethernet MAC and LAN8742 low-level interface.
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

#ifndef ETHERNET_LL_H
#define ETHERNET_LL_H

#include <stdint.h>

/**
  * @brief Result of Ethernet MAC and PHY initialization.
  */
typedef enum {
  ETHERNET_STATUS_OK = 0,
  ETHERNET_STATUS_INVALID_ARGUMENT,
  ETHERNET_STATUS_DMA_TIMEOUT,
  ETHERNET_STATUS_MDIO_TIMEOUT,
  ETHERNET_STATUS_PHY_TIMEOUT,
  ETHERNET_STATUS_PHY_MODE_ERROR,
} Ethernet_StatusTypeDef;

/**
  * @brief Initialize the RMII MAC, DMA descriptor rings, and LAN8742 PHY.
  * @param phyAddress (uint8_t) LAN8742 MDIO address from 0 through 31.
  * @param macAddress (const uint8_t[6]) Non-null six-byte unicast MAC address.
  * @retval (Ethernet_StatusTypeDef) Initialization result.
  * @note The `.eth` linker region must be configured as non-cacheable before
  *       this function runs.
  */
Ethernet_StatusTypeDef EthernetMac_Init(
  uint8_t phyAddress,
  const uint8_t macAddress[6]
);

#endif /* ETHERNET_LL_H */
