/**
  ******************************************************************************
  * @file           : network_interface.h
  * @brief          : lwIP adapter for the STM32F767 Ethernet MAC.
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

#ifndef NETWORK_INTERFACE_H
#define NETWORK_INTERFACE_H

#include "lwip/netif.h"

/**
  * @brief Bind an lwIP netif to the STM32F767 Ethernet frame functions.
  * @param networkInterface (struct netif*) Non-null interface owned by lwIP.
  * @retval ERR_OK Interface fields were configured.
  * @retval ERR_ARG The interface pointer was NULL.
  */
err_t NetworkInterface_Init(struct netif* networkInterface);

/**
  * @brief Drain pending DMA frames and submit copied pbufs to lwIP.
  * @param networkInterface (struct netif*) Initialized lwIP interface.
  * @note Called only by the network service task; does not block for frames.
  */
void NetworkInterface_ProcessInput(struct netif* networkInterface);

/**
  * @brief Synchronize the lwIP link flag with LAN8742 state.
  * @param networkInterface (struct netif*) Initialized lwIP interface.
  * @retval ERR_OK Link state was read and applied.
  * @retval ERR_ARG The interface pointer was NULL.
  * @retval ERR_IF PHY access or negotiated-mode application failed.
  */
err_t NetworkInterface_UpdateLink(struct netif* networkInterface);

#endif /* NETWORK_INTERFACE_H */
