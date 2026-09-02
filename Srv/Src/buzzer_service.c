/**
  ******************************************************************************
  * @file           : buzzer_service.c
  * @brief          : FreeRTOS-aware buzzer alert service.
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

#include "buzzer_service.h"

#include "buzzer.h"
#include "queue.h"
#include "task.h"

#define BUZZER_TASK_PRIORITY    (tskIDLE_PRIORITY + 1U)
#define BUZZER_STACK_WORDS      configMINIMAL_STACK_SIZE
#define BUZZER_QUEUE_LENGTH     1U
#define BUZZER_STARTUP_TONE_MS  150U
#define BUZZER_ALERT_TONE_MS    120U
#define BUZZER_ALERT_GAP_MS     120U
#define BUZZER_ALERT_BEEP_COUNT 3U
#define BUZZER_RESET_TONE_MS    350U
#define BUZZER_RESET_GAP_MS     150U
#define BUZZER_FAILURE_TONE_MS  70U
#define BUZZER_FAILURE_GAP_MS   70U
#define BUZZER_FAILURE_COUNT    5U
#define BUZZER_EVENT_ALERT      1U
#define BUZZER_EVENT_RESET      2U
#define BUZZER_EVENT_FAILURE    3U

static StaticTask_t buzzerTaskControlBlock;
static StackType_t buzzerTaskStack[BUZZER_STACK_WORDS];
static StaticQueue_t buzzerQueueControlBlock;
static uint8_t buzzerQueueStorage[BUZZER_QUEUE_LENGTH * sizeof(uint8_t)];
static QueueHandle_t buzzerQueue;

static void buzzerService_Task(void* argument);
static void buzzerService_Beep(uint32_t durationMs);

BaseType_t BuzzerService_Init(void) {
  buzzerQueue = xQueueCreateStatic(
    BUZZER_QUEUE_LENGTH,
    sizeof(uint8_t),
    buzzerQueueStorage,
    &buzzerQueueControlBlock
  );
  if (buzzerQueue == NULL)
    return pdFAIL;

  TaskHandle_t task = xTaskCreateStatic(
    buzzerService_Task,
    "buzzer",
    BUZZER_STACK_WORDS,
    NULL,
    BUZZER_TASK_PRIORITY,
    buzzerTaskStack,
    &buzzerTaskControlBlock
  );
  return (task != NULL) ? pdPASS : pdFAIL;
}

void BuzzerService_Alert(void) {
  if (buzzerQueue == NULL)
    return;
  uint8_t event = BUZZER_EVENT_ALERT;
  (void)xQueueSend(buzzerQueue, &event, 0U);
}

void BuzzerService_FactoryResetSignal(void) {
  if (buzzerQueue == NULL)
    return;
  uint8_t event = BUZZER_EVENT_RESET;
  (void)xQueueOverwrite(buzzerQueue, &event);
}

void BuzzerService_FactoryResetFailure(void) {
  if (buzzerQueue == NULL)
    return;
  uint8_t event = BUZZER_EVENT_FAILURE;
  (void)xQueueOverwrite(buzzerQueue, &event);
}

/**
  * @brief Play the startup self-test tone, then play the configured alert
  *        pattern once per queued request.
  * @param argument (void*) Unused task argument.
  */
static void buzzerService_Task(void* argument) {
  (void)argument;
  buzzerService_Beep(BUZZER_STARTUP_TONE_MS);

  for (;;) {
    uint8_t event;
    if (xQueueReceive(buzzerQueue, &event, portMAX_DELAY) != pdTRUE)
      continue;
    if (event == BUZZER_EVENT_RESET) {
      buzzerService_Beep(BUZZER_RESET_TONE_MS);
      vTaskDelay(pdMS_TO_TICKS(BUZZER_RESET_GAP_MS));
      buzzerService_Beep(BUZZER_RESET_TONE_MS);
      continue;
    }
    if (event == BUZZER_EVENT_FAILURE) {
      for (uint32_t index = 0U; index < BUZZER_FAILURE_COUNT; index++) {
        buzzerService_Beep(BUZZER_FAILURE_TONE_MS);
        if ((index + 1U) < BUZZER_FAILURE_COUNT)
          vTaskDelay(pdMS_TO_TICKS(BUZZER_FAILURE_GAP_MS));
      }
      continue;
    }
    for (uint32_t index = 0U; index < BUZZER_ALERT_BEEP_COUNT; index++) {
      buzzerService_Beep(BUZZER_ALERT_TONE_MS);
      if ((index + 1U) < BUZZER_ALERT_BEEP_COUNT)
        vTaskDelay(pdMS_TO_TICKS(BUZZER_ALERT_GAP_MS));
    }
  }
}

/**
  * @brief Sound the buzzer for one bounded duration, then silence it.
  * @param durationMs (uint32_t) Tone duration in milliseconds.
  */
static void buzzerService_Beep(uint32_t durationMs) {
  Buzzer_On();
  vTaskDelay(pdMS_TO_TICKS(durationMs));
  Buzzer_Off();
}
