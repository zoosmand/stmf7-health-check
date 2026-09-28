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

#include <stddef.h>
#include <stdint.h>

/**
  * @brief Result of Ethernet MAC and PHY initialization.
  */
typedef enum {
  ETHERNET_STATUS_OK = 0,             /**< Operation completed. */
  ETHERNET_STATUS_INVALID_ARGUMENT,   /**< Pointer, address, or length invalid. */
  ETHERNET_STATUS_DMA_TIMEOUT,        /**< DMA reset or operation timed out. */
  ETHERNET_STATUS_MDIO_TIMEOUT,       /**< PHY management transfer timed out. */
  ETHERNET_STATUS_PHY_NEGOTIATING,    /**< Autonegotiation is not complete. */
  ETHERNET_STATUS_PHY_MODE_ERROR,     /**< Negotiated mode is unsupported. */
  ETHERNET_STATUS_NO_FRAME,           /**< No received frame is pending. */
  ETHERNET_STATUS_FRAME_ERROR,        /**< DMA marked the frame invalid. */
  ETHERNET_STATUS_FRAME_TOO_LARGE,    /**< Frame exceeds one DMA buffer. */
  ETHERNET_STATUS_TRANSMIT_BUSY,      /**< Next TX descriptor belongs to DMA. */
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

/**
  * @brief Poll PHY state and apply an autonegotiated MAC mode.
  * @param linkUp (uint8_t*) Non-null destination; set to one only when the PHY
  *        link and autonegotiation are ready.
  * @retval (Ethernet_StatusTypeDef) Link-polling result.
  */
Ethernet_StatusTypeDef EthernetMac_UpdateLink(uint8_t* linkUp);

/**
  * @brief Copy and submit one contiguous Ethernet frame to DMA.
  * @param frame (const uint8_t*) Non-null frame bytes including Ethernet header.
  * @param length (size_t) Frame length from 1 through 1536 bytes.
  * @retval (Ethernet_StatusTypeDef) Submission result.
  */
Ethernet_StatusTypeDef EthernetMac_Transmit(
  const uint8_t* frame,
  size_t length
);

/**
  * @brief Borrow the next valid DMA receive frame without copying it.
  * @param frame (const uint8_t**) Destination for the DMA-buffer address.
  * @param length (size_t*) Destination for the frame length without CRC.
  * @retval (Ethernet_StatusTypeDef) Receive-ring result.
  * @note Call EthernetMac_ReleaseReceivedFrame() after copying the frame.
  */
Ethernet_StatusTypeDef EthernetMac_GetReceivedFrame(
  const uint8_t** frame,
  size_t* length
);

/**
  * @brief Return the current receive descriptor to DMA and advance the ring.
  */
void EthernetMac_ReleaseReceivedFrame(void);

/**
  * @brief Read the configured primary MAC address.
  * @param macAddress (uint8_t[6]) Non-null six-byte destination.
  */
void EthernetMac_GetAddress(uint8_t macAddress[6]);

#endif /* ETHERNET_LL_H */
