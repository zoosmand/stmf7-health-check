/**
  ******************************************************************************
  * @file           : factory_reset_service.c
  * @brief          : B1 long-press handling and persistent-state reset.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 02.09.2026
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

#include "factory_reset_service.h"

#include "buzzer_service.h"
#include "common.h"
#include "flash_layout.h"
#include "task.h"
#include "telemetry_service.h"
#include "user_button.h"

#define FACTORY_RESET_TASK_PRIORITY  (configMAX_PRIORITIES - 2U)
#define FACTORY_RESET_STACK_WORDS    256U
#define FACTORY_RESET_POLL_MS        20U
#define FACTORY_RESET_DEBOUNCE_MS    60U
#define FACTORY_RESET_HOLD_MS        10000U
#define FACTORY_RESET_HOLD_SAMPLES \
  (FACTORY_RESET_HOLD_MS / FACTORY_RESET_POLL_MS)
#define FACTORY_RESET_DEBOUNCE_SAMPLES \
  (FACTORY_RESET_DEBOUNCE_MS / FACTORY_RESET_POLL_MS)
#define FACTORY_RESET_CANCEL_WINDOW_MS 10000U
#define FACTORY_RESET_DOUBLE_CLICK_MS  600U
#define FACTORY_RESET_FAILURE_WAIT_MS 1000U
#define FACTORY_RESET_MARKER_MAGIC   0x46525354UL

typedef enum {
  FACTORY_RESET_BUTTON_EVENT_NONE = 0,
  FACTORY_RESET_BUTTON_EVENT_PRESSED,
  FACTORY_RESET_BUTTON_EVENT_RELEASED,
} FactoryReset_ButtonEventTypeDef;

typedef struct {
  uint8_t stablePressed;
  uint8_t candidatePressed;
  uint8_t candidateSamples;
} FactoryReset_ButtonStateTypeDef;

/** @brief Durable indication that persistent-state erasure must be completed. */
typedef struct {
  uint32_t magic;
  uint32_t inverseMagic;
} FactoryResetMarker_TypeDef;

static StaticTask_t factoryResetTaskControlBlock;
static StackType_t factoryResetTaskStack[FACTORY_RESET_STACK_WORDS];

static void factoryResetService_Task(void* argument);
static FactoryReset_ButtonEventTypeDef factoryResetService_UpdateButton(
  FactoryReset_ButtonStateTypeDef* button
);
static void factoryResetService_WaitForLongPress(
  FactoryReset_ButtonStateTypeDef* button
);
static uint8_t factoryResetService_WaitForCancellation(
  FactoryReset_ButtonStateTypeDef* button
);
static void factoryResetService_WaitForRelease(
  FactoryReset_ButtonStateTypeDef* button
);
static W25Q64_StatusTypeDef factoryResetService_ReadMarker(uint8_t* isValid);
static W25Q64_StatusTypeDef factoryResetService_WriteMarker(void);
static W25Q64_StatusTypeDef factoryResetService_ErasePersistentState(void);
static void factoryResetService_Restart(void) __attribute__((noreturn));

W25Q64_StatusTypeDef FactoryResetService_ResumePending(void) {
  uint8_t isValid = 0U;
  W25Q64_StatusTypeDef status = factoryResetService_ReadMarker(&isValid);
  if (status != W25Q64_STATUS_OK)
    return status;
  if (isValid == 0U)
    return W25Q64_STATUS_OK;

  Common_Printf("Factory reset: resuming interrupted reset.\n");
  status = factoryResetService_ErasePersistentState();
  if (status != W25Q64_STATUS_OK) {
    Common_Printf("Factory reset: recovery failed, status=%u.\n", status);
    return status;
  }

  Common_Printf("Factory reset: recovery complete; restarting.\n");
  NVIC_SystemReset();
  return W25Q64_STATUS_IO_ERROR;
}

BaseType_t FactoryResetService_Init(void) {
  TaskHandle_t task = xTaskCreateStatic(
    factoryResetService_Task,
    "factory-reset",
    FACTORY_RESET_STACK_WORDS,
    NULL,
    FACTORY_RESET_TASK_PRIORITY,
    factoryResetTaskStack,
    &factoryResetTaskControlBlock
  );
  if (task == NULL)
    return pdFAIL;
  TelemetryService_RegisterTask(TELEMETRY_TASK_FACTORY_RESET, task);
  return pdPASS;
}

static void factoryResetService_Task(void* argument) {
  (void)argument;
  FactoryReset_ButtonStateTypeDef button = {0U};
  for (;;) {
    factoryResetService_WaitForLongPress(&button);

    Common_Printf("Factory reset: long press confirmed.\n");
    BuzzerService_FactoryResetWarning();
    TickType_t warningStarted = xTaskGetTickCount();
    TickType_t warningWait = BuzzerService_FactoryResetWarningDuration()
      + pdMS_TO_TICKS(FACTORY_RESET_POLL_MS);
    while ((xTaskGetTickCount() - warningStarted)
        < warningWait) {
      (void)factoryResetService_UpdateButton(&button);
      vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_POLL_MS));
    }

    Common_Printf("Factory reset: cancellation window open.\n");
    if (factoryResetService_WaitForCancellation(&button) != 0U) {
      Common_Printf("Factory reset: cancelled by double-click.\n");
      BuzzerService_FactoryResetCancelled();
      factoryResetService_WaitForRelease(&button);
      continue;
    }

    Common_Printf("Factory reset: cancellation window expired.\n");
    W25Q64_StatusTypeDef status = factoryResetService_WriteMarker();
    if (status != W25Q64_STATUS_OK) {
      Common_Printf(
        "Factory reset: marker write failed, status=%u; reset cancelled.\n",
        status
      );
      BuzzerService_FactoryResetFailure();
      vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_FAILURE_WAIT_MS));
      factoryResetService_WaitForRelease(&button);
      continue;
    }

    Common_Printf("Factory reset: accepted; erasing persistent state.\n");
    status = factoryResetService_ErasePersistentState();
    if (status != W25Q64_STATUS_OK) {
      Common_Printf(
        "Factory reset: erase failed, status=%u; recovery pending.\n",
        status
      );
      BuzzerService_FactoryResetFailure();
      vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_FAILURE_WAIT_MS));
      factoryResetService_Restart();
    }

    Common_Printf("Factory reset: complete; restarting.\n");
    factoryResetService_Restart();
  }
}

/** @brief Debounce one raw B1 sample and return stable edge events. */
static FactoryReset_ButtonEventTypeDef factoryResetService_UpdateButton(
  FactoryReset_ButtonStateTypeDef* button
) {
  uint8_t pressed = UserButton_IsPressed() != 0U ? 1U : 0U;
  if (pressed != button->candidatePressed) {
    button->candidatePressed = pressed;
    button->candidateSamples = 1U;
    return FACTORY_RESET_BUTTON_EVENT_NONE;
  }
  if (button->candidateSamples < FACTORY_RESET_DEBOUNCE_SAMPLES)
    ++button->candidateSamples;
  if ((button->candidateSamples < FACTORY_RESET_DEBOUNCE_SAMPLES)
      || (button->stablePressed == button->candidatePressed)) {
    return FACTORY_RESET_BUTTON_EVENT_NONE;
  }
  button->stablePressed = button->candidatePressed;
  return button->stablePressed != 0U
    ? FACTORY_RESET_BUTTON_EVENT_PRESSED
    : FACTORY_RESET_BUTTON_EVENT_RELEASED;
}

/** @brief Wait for one uninterrupted ten-second debounced press. */
static void factoryResetService_WaitForLongPress(
  FactoryReset_ButtonStateTypeDef* button
) {
  uint32_t pressedSamples = 0U;
  while (pressedSamples < FACTORY_RESET_HOLD_SAMPLES) {
    (void)factoryResetService_UpdateButton(button);
    if (button->stablePressed != 0U)
      ++pressedSamples;
    else
      pressedSamples = 0U;
    vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_POLL_MS));
  }
}

/** @brief Detect a released-press-released-press double-click in the window. */
static uint8_t factoryResetService_WaitForCancellation(
  FactoryReset_ButtonStateTypeDef* button
) {
  TickType_t windowStarted = xTaskGetTickCount();
  TickType_t firstClickAt = 0U;
  uint8_t clickCount = 0U;
  uint8_t armed = button->stablePressed == 0U ? 1U : 0U;
  while ((xTaskGetTickCount() - windowStarted)
      < pdMS_TO_TICKS(FACTORY_RESET_CANCEL_WINDOW_MS)) {
    FactoryReset_ButtonEventTypeDef event =
      factoryResetService_UpdateButton(button);
    TickType_t now = xTaskGetTickCount();
    if (armed == 0U) {
      if (event == FACTORY_RESET_BUTTON_EVENT_RELEASED)
        armed = 1U;
    } else if (event == FACTORY_RESET_BUTTON_EVENT_PRESSED) {
      if ((clickCount == 0U)
          || ((now - firstClickAt)
            > pdMS_TO_TICKS(FACTORY_RESET_DOUBLE_CLICK_MS))) {
        clickCount = 1U;
        firstClickAt = now;
      } else {
        return 1U;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_POLL_MS));
  }
  return 0U;
}

/** @brief Prevent a completed gesture from becoming the next long press. */
static void factoryResetService_WaitForRelease(
  FactoryReset_ButtonStateTypeDef* button
) {
  while (button->stablePressed != 0U) {
    (void)factoryResetService_UpdateButton(button);
    vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_POLL_MS));
  }
}

static W25Q64_StatusTypeDef factoryResetService_ReadMarker(uint8_t* isValid) {
  FactoryResetMarker_TypeDef marker = {0U};
  W25Q64_StatusTypeDef status = W25Q64_Read(
    FLASH_LAYOUT_FACTORY_RESET_MARKER_SECTOR,
    &marker,
    sizeof(marker)
  );
  if (status != W25Q64_STATUS_OK)
    return status;
  *isValid = ((marker.magic == FACTORY_RESET_MARKER_MAGIC)
      && (marker.inverseMagic == ~FACTORY_RESET_MARKER_MAGIC))
    ? 1U
    : 0U;
  return W25Q64_STATUS_OK;
}

static W25Q64_StatusTypeDef factoryResetService_WriteMarker(void) {
  FactoryResetMarker_TypeDef marker = {
    .magic = FACTORY_RESET_MARKER_MAGIC,
    .inverseMagic = ~FACTORY_RESET_MARKER_MAGIC,
  };
  W25Q64_StatusTypeDef status = W25Q64_EraseSector(
    FLASH_LAYOUT_FACTORY_RESET_MARKER_SECTOR
  );
  if (status == W25Q64_STATUS_OK) {
    status = W25Q64_Program(
      FLASH_LAYOUT_FACTORY_RESET_MARKER_SECTOR,
      &marker,
      sizeof(marker)
    );
  }

  W25Q64_StatusTypeDef operationStatus = status;
  uint8_t isValid = 0U;
  status = factoryResetService_ReadMarker(&isValid);
  if ((status == W25Q64_STATUS_OK) && (isValid != 0U))
    return W25Q64_STATUS_OK;
  if (operationStatus != W25Q64_STATUS_OK)
    return operationStatus;
  return status != W25Q64_STATUS_OK ? status : W25Q64_STATUS_IO_ERROR;
}

static W25Q64_StatusTypeDef factoryResetService_ErasePersistentState(void) {
  W25Q64_StatusTypeDef status = W25Q64_EraseSector(
    FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_A
  );
  if (status != W25Q64_STATUS_OK)
    return status;
  status = W25Q64_EraseSector(FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_B);
  if (status != W25Q64_STATUS_OK)
    return status;
  return W25Q64_EraseRange(
    FLASH_LAYOUT_FACTORY_RESET_MARKER_SECTOR,
    FLASH_LAYOUT_FACTORY_RESET_DATA_LENGTH + W25Q64_SECTOR_SIZE
  );
}

static void factoryResetService_Restart(void) {
  taskENTER_CRITICAL();
  NVIC_SystemReset();
  for (;;) {
  }
}
