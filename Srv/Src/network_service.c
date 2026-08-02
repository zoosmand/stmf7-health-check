/**
  ******************************************************************************
  * @file           : network_service.c
  * @brief          : FreeRTOS lwIP DHCP and static-fallback management.
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

#include "network_service.h"

#include "common.h"
#include "network_interface.h"

#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/tcpip.h"

#include "task.h"

#include <stdio.h>

#define NETWORK_TASK_PRIORITY        (tskIDLE_PRIORITY + 2U)
#define NETWORK_TASK_STACK_WORDS     512U
#define NETWORK_POLL_PERIOD_MS       10U
#define NETWORK_LINK_POLL_PERIOD_MS  100U
#define NETWORK_DHCP_TIMEOUT_MS      10000U
#define NETWORK_INIT_TIMEOUT_MS      5000U

typedef enum {
  NETWORK_CONFIGURATION_NONE = 0, /**< No usable IPv4 parameters reported. */
  NETWORK_CONFIGURATION_DHCP,     /**< Active parameters came from DHCP. */
  NETWORK_CONFIGURATION_FALLBACK, /**< Documented static parameters active. */
} NetworkConfiguration_TypeDef;

static StaticTask_t networkTaskControlBlock;
static StackType_t networkTaskStack[NETWORK_TASK_STACK_WORDS];
static struct netif networkInterface;
static sys_sem_t networkInitSemaphore;
static volatile err_t networkInitResult;
static volatile uint8_t networkReady;
static uint32_t linkPollAt;
static uint32_t dhcpStartedAt;
static NetworkConfiguration_TypeDef activeConfiguration;

static void networkService_Task(void* argument);
static void networkService_CoreInit(void* argument);
static void networkService_CoreProcess(void* argument);
static void networkService_LinkChanged(struct netif* changedInterface);
static void networkService_StartDhcp(void);
static void networkService_ApplyFallback(void);
static void networkService_Report(const char* source);

BaseType_t NetworkService_Init(void) {
  TaskHandle_t task = xTaskCreateStatic(
    networkService_Task,
    "network",
    NETWORK_TASK_STACK_WORDS,
    NULL,
    NETWORK_TASK_PRIORITY,
    networkTaskStack,
    &networkTaskControlBlock
  );
  return (task != NULL) ? pdPASS : pdFAIL;
}

uint8_t NetworkService_IsReady(void) {
  return networkReady;
}

static void networkService_Task(void* argument) {
  (void)argument;
  Common_Printf("Network service: started.\n");
  if (sys_sem_new(&networkInitSemaphore, 0U) != ERR_OK)
    System_ErrorHandler();
  networkInitResult = ERR_IF;
  tcpip_init(networkService_CoreInit, NULL);
  if ((sys_arch_sem_wait(&networkInitSemaphore, NETWORK_INIT_TIMEOUT_MS)
        == SYS_ARCH_TIMEOUT)
      || (networkInitResult != ERR_OK)) {
    System_ErrorHandler();
  }
  sys_sem_free(&networkInitSemaphore);

  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    NetworkInterface_ProcessInput(&networkInterface);
    uint32_t now = sys_now();
    if ((now - linkPollAt) >= NETWORK_LINK_POLL_PERIOD_MS) {
      linkPollAt = now;
      (void)tcpip_callback_with_block(
        networkService_CoreProcess,
        NULL,
        1U
      );
    }
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(NETWORK_POLL_PERIOD_MS));
  }
}

static void networkService_CoreInit(void* argument) {
  (void)argument;
  ip4_addr_t any = {0};
  if (netif_add(
        &networkInterface,
        &any,
        &any,
        &any,
        NULL,
        NetworkInterface_Init,
        tcpip_input
      ) == NULL) {
    networkInitResult = ERR_IF;
    sys_sem_signal(&networkInitSemaphore);
    return;
  }
  netif_set_default(&networkInterface);
  netif_set_link_callback(&networkInterface, networkService_LinkChanged);
  networkReady = 0U;
  activeConfiguration = NETWORK_CONFIGURATION_NONE;
  dhcpStartedAt = sys_now();
  if (NetworkInterface_UpdateLink(&networkInterface) != ERR_OK) {
    networkInitResult = ERR_IF;
  } else {
    if (!netif_is_link_up(&networkInterface))
      netif_set_down(&networkInterface);
    networkInitResult = ERR_OK;
  }
  sys_sem_signal(&networkInitSemaphore);
}

static void networkService_CoreProcess(void* argument) {
  (void)argument;
  if (NetworkInterface_UpdateLink(&networkInterface) != ERR_OK)
    return;
  if (!netif_is_link_up(&networkInterface))
    return;
  uint32_t now = sys_now();
  if (dhcp_supplied_address(&networkInterface) != 0U) {
    if (activeConfiguration != NETWORK_CONFIGURATION_DHCP) {
      activeConfiguration = NETWORK_CONFIGURATION_DHCP;
      networkService_Report("DHCP");
    }
    return;
  }
  if ((activeConfiguration == NETWORK_CONFIGURATION_NONE)
      && ((now - dhcpStartedAt) >= NETWORK_DHCP_TIMEOUT_MS)) {
    networkService_ApplyFallback();
  }
}

static void networkService_LinkChanged(struct netif* changedInterface) {
  if (netif_is_link_up(changedInterface)) {
    netif_set_up(changedInterface);
    networkService_StartDhcp();
    return;
  }
  dhcp_stop(changedInterface);
  netif_set_down(changedInterface);
  dhcpStartedAt = sys_now();
  activeConfiguration = NETWORK_CONFIGURATION_NONE;
  networkReady = 0U;
}

static void networkService_StartDhcp(void) {
  netif_set_addr(
    &networkInterface,
    IP4_ADDR_ANY4,
    IP4_ADDR_ANY4,
    IP4_ADDR_ANY4
  );
  dns_setserver(0U, IP_ADDR_ANY);
  dhcpStartedAt = sys_now();
  activeConfiguration = NETWORK_CONFIGURATION_NONE;
  networkReady = 0U;
  if (dhcp_start(&networkInterface) != ERR_OK)
    networkService_ApplyFallback();
}

static void networkService_ApplyFallback(void) {
  ip4_addr_t address;
  ip4_addr_t netmask;
  ip4_addr_t gateway;
  ip_addr_t dnsServer;
  IP4_ADDR(&address, 192U, 168U, 0U, 50U);
  IP4_ADDR(&netmask, 255U, 255U, 255U, 0U);
  IP4_ADDR(&gateway, 192U, 168U, 0U, 1U);
  IP_ADDR4(&dnsServer, 8U, 8U, 8U, 8U);
  if (!netif_is_link_up(&networkInterface))
    dhcp_stop(&networkInterface);
  netif_set_addr(&networkInterface, &address, &netmask, &gateway);
  dns_setserver(0U, &dnsServer);
  activeConfiguration = NETWORK_CONFIGURATION_FALLBACK;
  networkService_Report("static fallback");
}

static void networkService_Report(const char* source) {
  const ip4_addr_t* address = netif_ip4_addr(&networkInterface);
  const ip4_addr_t* netmask = netif_ip4_netmask(&networkInterface);
  const ip4_addr_t* gateway = netif_ip4_gw(&networkInterface);
  const ip4_addr_t* dnsAddress = ip_2_ip4(dns_getserver(0U));
  char addressText[IP4ADDR_STRLEN_MAX];
  char netmaskText[IP4ADDR_STRLEN_MAX];
  char gatewayText[IP4ADDR_STRLEN_MAX];
  char dnsText[IP4ADDR_STRLEN_MAX];
  (void)ip4addr_ntoa_r(address, addressText, sizeof(addressText));
  (void)ip4addr_ntoa_r(netmask, netmaskText, sizeof(netmaskText));
  (void)ip4addr_ntoa_r(gateway, gatewayText, sizeof(gatewayText));
  (void)ip4addr_ntoa_r(dnsAddress, dnsText, sizeof(dnsText));
  networkReady = netif_is_link_up(&networkInterface) ? 1U : 0U;
  Common_Printf(
    "ETH configuration: %s\n"
    "ETH IP: %s\n"
    "ETH MASK: %s\n"
    "ETH GATE: %s\n"
    "ETH DNS: %s\n",
    source,
    addressText,
    netmaskText,
    gatewayText,
    dnsText
  );
}
