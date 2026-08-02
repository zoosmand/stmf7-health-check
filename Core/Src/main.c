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
  Gpio_InitOutput(GPIOB, HEART_BEAT_PIN_POSITION);
  GPIO_PIN_RESET(GPIOB, 1UL << HEART_BEAT_PIN_POSITION);

  Ethernet_StatusTypeDef ethernetStatus = Board_InitEthernet();
  if (ethernetStatus != ETHERNET_STATUS_OK) {
    printf("Ethernet initialization failed: %u\n", (unsigned)ethernetStatus);
  }

  if (HeartBeatService_Init() != pdPASS)
    System_ErrorHandler();

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
