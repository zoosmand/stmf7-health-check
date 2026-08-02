/**
  ******************************************************************************
  * @file           : heart_beat.c
  * @brief          : FreeRTOS heartbeat LED service.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 05.10.2025
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

#include "heart_beat.h"

#include "common.h"
#include "task.h"

#define HEART_BEAT_PORT           GPIOB
#define HEART_BEAT_PIN_MASK       (1UL << 14U)
#define HEART_BEAT_PERIOD_MS      1500U
#define HEART_BEAT_PULSE_MS       150U
#define HEART_BEAT_TASK_PRIORITY  (tskIDLE_PRIORITY + 1U)
#define HEART_BEAT_STACK_WORDS    configMINIMAL_STACK_SIZE

static StaticTask_t heartBeatTaskControlBlock;
static StackType_t heartBeatTaskStack[HEART_BEAT_STACK_WORDS];

static void heartBeatService_Task(void* argument);

BaseType_t HeartBeatService_Init(void) {
  TaskHandle_t task = xTaskCreateStatic(
    heartBeatService_Task,
    "heartbeat",
    HEART_BEAT_STACK_WORDS,
    NULL,
    HEART_BEAT_TASK_PRIORITY,
    heartBeatTaskStack,
    &heartBeatTaskControlBlock
  );
  return (task != NULL) ? pdPASS : pdFAIL;
}

/**
  * @brief Emit two short LED pulses once per heartbeat period.
  * @param argument (void*) Unused task argument.
  */
static void heartBeatService_Task(void* argument) {
  (void)argument;
  const TickType_t pulseTicks = pdMS_TO_TICKS(HEART_BEAT_PULSE_MS);
  const TickType_t quietTicks = pdMS_TO_TICKS(
    HEART_BEAT_PERIOD_MS - (3U * HEART_BEAT_PULSE_MS)
  );

  for (;;) {
    GPIO_PIN_SET(HEART_BEAT_PORT, HEART_BEAT_PIN_MASK);
    vTaskDelay(pulseTicks);
    GPIO_PIN_RESET(HEART_BEAT_PORT, HEART_BEAT_PIN_MASK);
    vTaskDelay(pulseTicks);
    GPIO_PIN_SET(HEART_BEAT_PORT, HEART_BEAT_PIN_MASK);
    vTaskDelay(pulseTicks);
    GPIO_PIN_RESET(HEART_BEAT_PORT, HEART_BEAT_PIN_MASK);
    vTaskDelay(quietTicks);
  }
}
