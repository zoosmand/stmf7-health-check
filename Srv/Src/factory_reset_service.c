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
#include "user_button.h"

#define FACTORY_RESET_TASK_PRIORITY  (tskIDLE_PRIORITY + 1U)
#define FACTORY_RESET_STACK_WORDS    256U
#define FACTORY_RESET_POLL_MS        20U
#define FACTORY_RESET_HOLD_MS        5000U
#define FACTORY_RESET_SIGNAL_WAIT_MS 1200U
#define FACTORY_RESET_MARKER_MAGIC   0x46525354UL

/** @brief Durable indication that persistent-state erasure must be completed. */
typedef struct {
  uint32_t magic;
  uint32_t inverseMagic;
} FactoryResetMarker_TypeDef;

static StaticTask_t factoryResetTaskControlBlock;
static StackType_t factoryResetTaskStack[FACTORY_RESET_STACK_WORDS];

static void factoryResetService_Task(void* argument);
static W25Q64_StatusTypeDef factoryResetService_ReadMarker(uint8_t* isValid);
static W25Q64_StatusTypeDef factoryResetService_WriteMarker(void);
static W25Q64_StatusTypeDef factoryResetService_ErasePersistentState(void);

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
  return (task != NULL) ? pdPASS : pdFAIL;
}

static void factoryResetService_Task(void* argument) {
  (void)argument;
  for (;;) {
    while (UserButton_IsPressed() == 0U)
      vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_POLL_MS));

    TickType_t pressStarted = xTaskGetTickCount();
    while ((UserButton_IsPressed() != 0U)
        && ((xTaskGetTickCount() - pressStarted)
          < pdMS_TO_TICKS(FACTORY_RESET_HOLD_MS))) {
      vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_POLL_MS));
    }
    if (UserButton_IsPressed() == 0U)
      continue;

    Common_Printf("Factory reset: long press confirmed.\n");
    BuzzerService_FactoryResetSignal();
    vTaskDelay(pdMS_TO_TICKS(FACTORY_RESET_SIGNAL_WAIT_MS));
    W25Q64_StatusTypeDef status = factoryResetService_WriteMarker();
    if (status == W25Q64_STATUS_OK)
      status = factoryResetService_ErasePersistentState();
    if (status == W25Q64_STATUS_OK) {
      Common_Printf("Factory reset: complete; restarting.\n");
      NVIC_SystemReset();
    }

    Common_Printf("Factory reset: failed, status=%u; recovery pending.\n", status);
    NVIC_SystemReset();
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
  if (status != W25Q64_STATUS_OK)
    return status;
  uint8_t isValid = 0U;
  status = factoryResetService_ReadMarker(&isValid);
  return ((status == W25Q64_STATUS_OK) && (isValid == 0U))
    ? W25Q64_STATUS_IO_ERROR
    : status;
}

static W25Q64_StatusTypeDef factoryResetService_ErasePersistentState(void) {
  return W25Q64_EraseRange(
    FLASH_LAYOUT_FACTORY_RESET_MARKER_SECTOR,
    FLASH_LAYOUT_FACTORY_RESET_DATA_LENGTH + W25Q64_SECTOR_SIZE
  );
}
