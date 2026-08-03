/**
  ******************************************************************************
  * @file           : watchdog_service.c
  * @brief          : Independent watchdog (IWDG) supervision service.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 03.08.2026
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

#include "watchdog_service.h"

#include "common.h"
#include "task.h"

#define WATCHDOG_TASK_PRIORITY (configMAX_PRIORITIES - 1U)
#define WATCHDOG_STACK_WORDS   configMINIMAL_STACK_SIZE
#define WATCHDOG_FEED_MS       750U

static StaticTask_t watchdogTaskControlBlock;
static StackType_t watchdogTaskStack[WATCHDOG_STACK_WORDS];

static void watchdogService_Task(void* argument);
static void watchdogService_Start(void);

BaseType_t WatchdogService_Init(void) {
  TaskHandle_t task = xTaskCreateStatic(
    watchdogService_Task,
    "watchdog",
    WATCHDOG_STACK_WORDS,
    NULL,
    WATCHDOG_TASK_PRIORITY,
    watchdogTaskStack,
    &watchdogTaskControlBlock
  );
  return (task != NULL) ? pdPASS : pdFAIL;
}

/**
  * @brief Start the IWDG once the scheduler is running, then reload it once
  *        per feed period for as long as the task keeps getting scheduled.
  * @param argument (void*) Unused task argument.
  */
static void watchdogService_Task(void* argument) {
  (void)argument;
  watchdogService_Start();

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(WATCHDOG_FEED_MS));
    IWDG->KR = IWDG_KEY_RELOAD;
  }
}

/**
  * @brief Configure and enable the IWDG for a nominal 2.4 s timeout.
  * @note Irreversible: once enabled, the IWDG cannot be disabled until reset.
  */
static void watchdogService_Start(void) {
  IWDG->KR = IWDG_KEY_ENABLE;
  IWDG->KR = IWDG_KEY_ACCESS;
  IWDG->PR = IWDG_PRESCALER_DIV64;
  IWDG->RLR = IWDG_RELOAD_COUNTER;
  while (IWDG->SR != 0U);
  IWDG->KR = IWDG_KEY_RELOAD;
}
