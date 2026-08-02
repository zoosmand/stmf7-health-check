/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Application entry point and baseline service startup.
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

#include "main.h"

#include <stdio.h>

#define HEART_BEAT_PIN_POSITION 14U

int main(void) {
  Board_InitDiagnosticUart();
  Board_PrintConfiguration();

  printf("Startup: initializing W25Q64 Flash...\n");
  Spi1_Init();
  W25Q64_StatusTypeDef flashStatus = W25Q64_Init();
  if (flashStatus == W25Q64_STATUS_OK)
    printf("Startup: W25Q64 Flash ready.\n");
  else
    printf("W25Q64 initialization failed: %u\n", (unsigned)flashStatus);

  printf("Startup: initializing heartbeat output...\n");
  Gpio_InitOutput(GPIOB, HEART_BEAT_PIN_POSITION);
  GPIO_PIN_RESET(GPIOB, 1UL << HEART_BEAT_PIN_POSITION);
  printf("Startup: heartbeat output ready.\n");

  printf("Startup: initializing Ethernet MAC and PHY...\n");
  Ethernet_StatusTypeDef ethernetStatus = Board_InitEthernet();
  if (ethernetStatus == ETHERNET_STATUS_OK)
    printf("Startup: Ethernet MAC and PHY ready.\n");
  else
    printf("Ethernet initialization failed: %u\n", (unsigned)ethernetStatus);

  printf("Startup: creating heartbeat service...\n");
  if (HeartBeatService_Init() != pdPASS)
    System_ErrorHandler();
  printf("Startup: heartbeat service ready.\n");

  if (ethernetStatus == ETHERNET_STATUS_OK) {
    printf("Startup: creating network service...\n");
    if (NetworkService_Init() != pdPASS)
      System_ErrorHandler();
    printf("Startup: network service ready.\n");
  }

  printf("Startup: starting FreeRTOS scheduler.\n");
  vTaskStartScheduler();
  System_ErrorHandler();
}

#if (configCHECK_FOR_STACK_OVERFLOW > 0)
void vApplicationStackOverflowHook(
  TaskHandle_t task,
  char* taskName
) {
  (void)task;
  (void)taskName;
  System_ErrorHandler();
}
#endif
