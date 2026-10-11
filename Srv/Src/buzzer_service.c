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
#include "telemetry_service.h"

#define BUZZER_TASK_PRIORITY    (tskIDLE_PRIORITY + 1U)
#define BUZZER_STACK_WORDS      configMINIMAL_STACK_SIZE
#define BUZZER_QUEUE_LENGTH     1U
#define BUZZER_STARTUP_TONE_MS  150U
#define BUZZER_ALERT_TONE_MS    120U
#define BUZZER_ALERT_GAP_MS     120U
#define BUZZER_RESET_TONE_MS    350U
#define BUZZER_RESET_GAP_MS     150U
#define BUZZER_FAILURE_TONE_MS  70U
#define BUZZER_FAILURE_GAP_MS   70U
#define BUZZER_FAILURE_COUNT    5U
#define BUZZER_DEFAULT_TONE_HZ  2500U
#define BUZZER_MELODY_TONE_MS   120U
#define BUZZER_MELODY_GAP_MS    45U
#define BUZZER_MELODY_NOTES     3U
#define BUZZER_WARNING_PAUSE_MS 250U
#define BUZZER_SOS_DOT_MS       100U
#define BUZZER_SOS_DASH_MS      300U
#define BUZZER_SOS_SYMBOL_GAP_MS 100U
#define BUZZER_SOS_LETTER_GAP_MS 300U
#define BUZZER_SOS_ID_PAUSE_MS   700U
#define BUZZER_SOS_TONE_HZ      1800U
#define BUZZER_EVENT_ALERT           1U
#define BUZZER_EVENT_RESET_WARNING   2U
#define BUZZER_EVENT_RESET_CANCELLED 3U
#define BUZZER_EVENT_FAILURE         4U
#define BUZZER_EVENT_CERTIFICATE     5U
#define BUZZER_RESET_WARNING_COUNT   5U
#define BUZZER_RESET_CANCEL_COUNT    3U

static StaticTask_t buzzerTaskControlBlock;
static StackType_t buzzerTaskStack[BUZZER_STACK_WORDS];
static StaticQueue_t buzzerQueueControlBlock;
typedef struct {
  uint8_t type;
  uint8_t count;
} BuzzerService_EventTypeDef;
static uint8_t buzzerQueueStorage[
  BUZZER_QUEUE_LENGTH * sizeof(BuzzerService_EventTypeDef)
];
static QueueHandle_t buzzerQueue;

static void buzzerService_Task(void* argument);
static void buzzerService_Beep(uint32_t durationMs);
static void buzzerService_PlayResourceWarning(
  const uint32_t melodyHz[BUZZER_MELODY_NOTES],
  uint8_t resourceId
);
static void buzzerService_PlayHealthCheckAlert(uint8_t resourceId);
static void buzzerService_PlayResourceId(uint8_t resourceId);

BaseType_t BuzzerService_Init(void) {
  buzzerQueue = xQueueCreateStatic(
    BUZZER_QUEUE_LENGTH,
    sizeof(BuzzerService_EventTypeDef),
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
  if (task == NULL)
    return pdFAIL;
  TelemetryService_RegisterTask(TELEMETRY_TASK_BUZZER, task);
  return pdPASS;
}

BaseType_t BuzzerService_HealthCheckAlert(uint8_t resourceId) {
  if ((buzzerQueue == NULL) || (resourceId == 0U))
    return pdFAIL;
  BuzzerService_EventTypeDef event = {
    .type = BUZZER_EVENT_ALERT,
    .count = resourceId,
  };
  return xQueueSend(buzzerQueue, &event, 0U);
}

BaseType_t BuzzerService_CertificateWarning(uint8_t resourceId) {
  if ((buzzerQueue == NULL) || (resourceId == 0U))
    return pdFAIL;
  BuzzerService_EventTypeDef event = {
    .type = BUZZER_EVENT_CERTIFICATE,
    .count = resourceId,
  };
  return xQueueSend(buzzerQueue, &event, 0U);
}

void BuzzerService_FactoryResetWarning(void) {
  if (buzzerQueue == NULL)
    return;
  BuzzerService_EventTypeDef event = { .type = BUZZER_EVENT_RESET_WARNING };
  (void)xQueueOverwrite(buzzerQueue, &event);
}

TickType_t BuzzerService_FactoryResetWarningDuration(void) {
  return (BUZZER_RESET_WARNING_COUNT * pdMS_TO_TICKS(BUZZER_RESET_TONE_MS))
    + ((BUZZER_RESET_WARNING_COUNT - 1U)
      * pdMS_TO_TICKS(BUZZER_RESET_GAP_MS));
}

void BuzzerService_FactoryResetCancelled(void) {
  if (buzzerQueue == NULL)
    return;
  BuzzerService_EventTypeDef event = { .type = BUZZER_EVENT_RESET_CANCELLED };
  (void)xQueueOverwrite(buzzerQueue, &event);
}

void BuzzerService_FactoryResetFailure(void) {
  if (buzzerQueue == NULL)
    return;
  BuzzerService_EventTypeDef event = { .type = BUZZER_EVENT_FAILURE };
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
    BuzzerService_EventTypeDef event;
    if (xQueueReceive(buzzerQueue, &event, portMAX_DELAY) != pdTRUE)
      continue;
    if ((event.type == BUZZER_EVENT_RESET_WARNING)
        || (event.type == BUZZER_EVENT_RESET_CANCELLED)) {
      uint32_t count = event.type == BUZZER_EVENT_RESET_WARNING
        ? BUZZER_RESET_WARNING_COUNT : BUZZER_RESET_CANCEL_COUNT;
      for (uint32_t index = 0U; index < count; ++index) {
        buzzerService_Beep(BUZZER_RESET_TONE_MS);
        if ((index + 1U) < count)
          vTaskDelay(pdMS_TO_TICKS(BUZZER_RESET_GAP_MS));
      }
      continue;
    }
    if (event.type == BUZZER_EVENT_FAILURE) {
      for (uint32_t index = 0U; index < BUZZER_FAILURE_COUNT; index++) {
        buzzerService_Beep(BUZZER_FAILURE_TONE_MS);
        if ((index + 1U) < BUZZER_FAILURE_COUNT)
          vTaskDelay(pdMS_TO_TICKS(BUZZER_FAILURE_GAP_MS));
      }
      continue;
    }
    if (event.type == BUZZER_EVENT_CERTIFICATE) {
      static const uint32_t certificateMelodyHz[BUZZER_MELODY_NOTES] = {
        1760U, 1175U, 1760U,
      };
      buzzerService_PlayResourceWarning(certificateMelodyHz, event.count);
      continue;
    }
    buzzerService_PlayHealthCheckAlert(event.count);
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

static void buzzerService_PlayResourceWarning(
  const uint32_t melodyHz[BUZZER_MELODY_NOTES],
  uint8_t resourceId
) {
  for (uint32_t index = 0U; index < BUZZER_MELODY_NOTES; ++index) {
    Buzzer_SetFrequency(melodyHz[index]);
    buzzerService_Beep(BUZZER_MELODY_TONE_MS);
    if ((index + 1U) < BUZZER_MELODY_NOTES)
      vTaskDelay(pdMS_TO_TICKS(BUZZER_MELODY_GAP_MS));
  }
  vTaskDelay(pdMS_TO_TICKS(BUZZER_WARNING_PAUSE_MS));
  buzzerService_PlayResourceId(resourceId);
}

static void buzzerService_PlayHealthCheckAlert(uint8_t resourceId) {
  static const uint32_t durationsMs[9] = {
    BUZZER_SOS_DOT_MS, BUZZER_SOS_DOT_MS, BUZZER_SOS_DOT_MS,
    BUZZER_SOS_DASH_MS, BUZZER_SOS_DASH_MS, BUZZER_SOS_DASH_MS,
    BUZZER_SOS_DOT_MS, BUZZER_SOS_DOT_MS, BUZZER_SOS_DOT_MS,
  };
  Buzzer_SetFrequency(BUZZER_SOS_TONE_HZ);
  for (uint32_t index = 0U; index < 9U; ++index) {
    buzzerService_Beep(durationsMs[index]);
    if ((index == 2U) || (index == 5U)) {
      vTaskDelay(pdMS_TO_TICKS(BUZZER_SOS_LETTER_GAP_MS));
    } else if ((index + 1U) < 9U) {
      vTaskDelay(pdMS_TO_TICKS(BUZZER_SOS_SYMBOL_GAP_MS));
    }
  }
  vTaskDelay(pdMS_TO_TICKS(BUZZER_SOS_ID_PAUSE_MS));
  buzzerService_PlayResourceId(resourceId);
}

static void buzzerService_PlayResourceId(uint8_t resourceId) {
  Buzzer_SetFrequency(BUZZER_DEFAULT_TONE_HZ);
  for (uint32_t index = 0U; index < resourceId; ++index) {
    buzzerService_Beep(BUZZER_ALERT_TONE_MS);
    if ((index + 1U) < resourceId)
      vTaskDelay(pdMS_TO_TICKS(BUZZER_ALERT_GAP_MS));
  }
}
