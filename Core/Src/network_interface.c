/**
  ******************************************************************************
  * @file           : network_interface.c
  * @brief          : Polling lwIP frame and link adapter for STM32F767 Ethernet.
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

#include "network_interface.h"

#include "ethernet_ll.h"

#include "lwip/etharp.h"
#include "lwip/pbuf.h"
#include "lwip/tcpip.h"

#define NETWORK_INTERFACE_MTU        1500U
#define NETWORK_INTERFACE_FRAME_SIZE 1536U

static uint8_t transmitFrame[NETWORK_INTERFACE_FRAME_SIZE];

static err_t networkInterface_Output(
  struct netif* networkInterface,
  struct pbuf* packet
);

err_t NetworkInterface_Init(struct netif* networkInterface) {
  if (networkInterface == NULL)
    return ERR_ARG;
  networkInterface->name[0] = 'e';
  networkInterface->name[1] = 'n';
  networkInterface->output = etharp_output;
  networkInterface->linkoutput = networkInterface_Output;
  networkInterface->mtu = NETWORK_INTERFACE_MTU;
  networkInterface->hwaddr_len = ETH_HWADDR_LEN;
  EthernetMac_GetAddress(networkInterface->hwaddr);
  networkInterface->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
  return ERR_OK;
}

void NetworkInterface_ProcessInput(struct netif* networkInterface) {
  if (networkInterface == NULL)
    return;
  for (;;) {
    const uint8_t* frame = NULL;
    size_t length = 0U;
    Ethernet_StatusTypeDef status = EthernetMac_GetReceivedFrame(
      &frame, &length
    );
    if (status == ETHERNET_STATUS_NO_FRAME)
      return;
    if (status != ETHERNET_STATUS_OK)
      continue;
    struct pbuf* packet = pbuf_alloc(PBUF_RAW, (u16_t)length, PBUF_POOL);
    if (packet != NULL) {
      if ((pbuf_take(packet, frame, (u16_t)length) != ERR_OK)
          || (networkInterface->input(packet, networkInterface) != ERR_OK)) {
        pbuf_free(packet);
      }
    }
    EthernetMac_ReleaseReceivedFrame();
  }
}

err_t NetworkInterface_UpdateLink(struct netif* networkInterface) {
  if (networkInterface == NULL)
    return ERR_ARG;
  uint8_t linkUp = 0U;
  if (EthernetMac_UpdateLink(&linkUp) != ETHERNET_STATUS_OK)
    return ERR_IF;
  if (linkUp != 0U) {
    if (!netif_is_link_up(networkInterface))
      netif_set_link_up(networkInterface);
  } else if (netif_is_link_up(networkInterface)) {
    netif_set_link_down(networkInterface);
  }
  return ERR_OK;
}

static err_t networkInterface_Output(
  struct netif* networkInterface,
  struct pbuf* packet
) {
  (void)networkInterface;
  if ((packet == NULL) || (packet->tot_len > sizeof(transmitFrame)))
    return ERR_ARG;
  if (pbuf_copy_partial(packet, transmitFrame, packet->tot_len, 0U)
      != packet->tot_len) {
    return ERR_IF;
  }
  Ethernet_StatusTypeDef status = EthernetMac_Transmit(
    transmitFrame, packet->tot_len
  );
  return (status == ETHERNET_STATUS_OK) ? ERR_OK : ERR_IF;
}
