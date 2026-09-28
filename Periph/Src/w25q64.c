/**
  ******************************************************************************
  * @file           : w25q64.c
  * @brief          : Bounded and serialized W25Q64 NOR Flash access.
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

#include "w25q64.h"

#include "common.h"
#include "spi.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define W25Q64_COMMAND_JEDEC_ID       0x9FU
#define W25Q64_COMMAND_READ           0x03U
#define W25Q64_COMMAND_WRITE_ENABLE   0x06U
#define W25Q64_COMMAND_STATUS         0x05U
#define W25Q64_COMMAND_PAGE_PROGRAM   0x02U
#define W25Q64_COMMAND_SECTOR_ERASE   0x20U
#define W25Q64_STATUS_BUSY            0x01U
#define W25Q64_PROGRAM_TIMEOUT_MS     100U
#define W25Q64_ERASE_TIMEOUT_MS       2000U
#define W25Q64_CHIP_SELECT_PIN_MASK   (1UL << 4U)

static StaticSemaphore_t flashMutexControlBlock;
static SemaphoreHandle_t flashMutex;

static void w25q64_Select(void);
static void w25q64_Deselect(void);
static W25Q64_StatusTypeDef w25q64_Command(
  const uint8_t* command,
  size_t commandLength,
  uint8_t* receive,
  size_t receiveLength
);
static W25Q64_StatusTypeDef w25q64_WriteEnable(void);
static W25Q64_StatusTypeDef w25q64_WaitReady(uint32_t timeoutMs);
static W25Q64_StatusTypeDef w25q64_EraseSectorUnlocked(uint32_t address);
static uint8_t w25q64_IsRangeValid(uint32_t address, size_t length);

/**
  * @brief Initialize serialized Flash access and perform a read-only probe.
  * @retval (W25Q64_StatusTypeDef) Initialization and identity-check result.
  */
W25Q64_StatusTypeDef W25Q64_Init(void) {
  if (flashMutex == NULL) {
    flashMutex = xSemaphoreCreateMutexStatic(&flashMutexControlBlock);
    if (flashMutex == NULL)
      return W25Q64_STATUS_IO_ERROR;
  }
  return W25Q64_IsAvailable() != 0U
    ? W25Q64_STATUS_OK
    : W25Q64_STATUS_UNAVAILABLE;
}

/**
  * @brief Read and validate the three-byte JEDEC identity under the mutex.
  * @retval (uint8_t) One for EF 40 17; otherwise zero.
  */
uint8_t W25Q64_IsAvailable(void) {
  if ((flashMutex == NULL)
      || (xSemaphoreTake(flashMutex, portMAX_DELAY) != pdTRUE)) {
    return 0U;
  }

  const uint8_t command = W25Q64_COMMAND_JEDEC_ID;
  uint8_t identity[3] = {0U};
  W25Q64_StatusTypeDef status = w25q64_Command(
    &command,
    1U,
    identity,
    sizeof(identity)
  );
  (void)xSemaphoreGive(flashMutex);
  return ((status == W25Q64_STATUS_OK)
      && (identity[0] == 0xEFU)
      && (identity[1] == 0x40U)
      && (identity[2] == 0x17U))
    ? 1U
    : 0U;
}

/**
  * @brief Read a validated Flash range while holding exclusive ownership.
  * @param address (uint32_t) First byte address.
  * @param data (void*) Destination buffer.
  * @param length (size_t) Number of bytes to read.
  * @retval (W25Q64_StatusTypeDef) Read result.
  */
W25Q64_StatusTypeDef W25Q64_Read(
  uint32_t address,
  void* data,
  size_t length
) {
  if ((data == NULL) || (w25q64_IsRangeValid(address, length) == 0U))
    return W25Q64_STATUS_INVALID_ARGUMENT;
  if ((flashMutex == NULL)
      || (xSemaphoreTake(flashMutex, portMAX_DELAY) != pdTRUE)) {
    return W25Q64_STATUS_IO_ERROR;
  }

  uint8_t command[4] = {
    W25Q64_COMMAND_READ,
    (uint8_t)(address >> 16U),
    (uint8_t)(address >> 8U),
    (uint8_t)address,
  };
  W25Q64_StatusTypeDef status = w25q64_Command(
    command,
    sizeof(command),
    data,
    length
  );
  (void)xSemaphoreGive(flashMutex);
  return status;
}

/**
  * @brief Issue write-enable and erase one validated sector.
  * @param address (uint32_t) Aligned 4 KiB sector address.
  * @retval (W25Q64_StatusTypeDef) Erase result after waiting for readiness.
  */
W25Q64_StatusTypeDef W25Q64_EraseSector(uint32_t address) {
  if ((address >= W25Q64_CAPACITY_BYTES)
      || ((address % W25Q64_SECTOR_SIZE) != 0U)) {
    return W25Q64_STATUS_INVALID_ARGUMENT;
  }
  if ((flashMutex == NULL)
      || (xSemaphoreTake(flashMutex, portMAX_DELAY) != pdTRUE)) {
    return W25Q64_STATUS_IO_ERROR;
  }

  W25Q64_StatusTypeDef status = w25q64_EraseSectorUnlocked(address);
  (void)xSemaphoreGive(flashMutex);
  return status;
}

W25Q64_StatusTypeDef W25Q64_EraseRange(uint32_t address, size_t length) {
  if ((length == 0U)
      || ((address % W25Q64_SECTOR_SIZE) != 0U)
      || ((length % W25Q64_SECTOR_SIZE) != 0U)
      || (w25q64_IsRangeValid(address, length) == 0U)) {
    return W25Q64_STATUS_INVALID_ARGUMENT;
  }
  if ((flashMutex == NULL)
      || (xSemaphoreTake(flashMutex, portMAX_DELAY) != pdTRUE)) {
    return W25Q64_STATUS_IO_ERROR;
  }

  W25Q64_StatusTypeDef status = W25Q64_STATUS_OK;
  const uint32_t firstAddress = address;
  uint32_t currentAddress = address + (uint32_t)length - W25Q64_SECTOR_SIZE;
  while (status == W25Q64_STATUS_OK) {
    status = w25q64_EraseSectorUnlocked(currentAddress);
    if ((status != W25Q64_STATUS_OK) || (currentAddress == firstAddress))
      break;
    currentAddress -= W25Q64_SECTOR_SIZE;
  }
  (void)xSemaphoreGive(flashMutex);
  return status;
}

static W25Q64_StatusTypeDef w25q64_EraseSectorUnlocked(uint32_t address) {
  W25Q64_StatusTypeDef status = w25q64_WriteEnable();
  uint8_t command[4] = {
    W25Q64_COMMAND_SECTOR_ERASE,
    (uint8_t)(address >> 16U),
    (uint8_t)(address >> 8U),
    (uint8_t)address,
  };
  if (status == W25Q64_STATUS_OK)
    status = w25q64_Command(command, sizeof(command), NULL, 0U);
  if (status == W25Q64_STATUS_OK)
    status = w25q64_WaitReady(W25Q64_ERASE_TIMEOUT_MS);
  return status;
}

/**
  * @brief Split and program a validated range at page boundaries.
  * @param address (uint32_t) First byte address.
  * @param data (const void*) Source buffer.
  * @param length (size_t) Number of bytes to program.
  * @retval (W25Q64_StatusTypeDef) Result of all page-program operations.
  */
W25Q64_StatusTypeDef W25Q64_Program(
  uint32_t address,
  const void* data,
  size_t length
) {
  if ((data == NULL) || (w25q64_IsRangeValid(address, length) == 0U))
    return W25Q64_STATUS_INVALID_ARGUMENT;
  if ((flashMutex == NULL)
      || (xSemaphoreTake(flashMutex, portMAX_DELAY) != pdTRUE)) {
    return W25Q64_STATUS_IO_ERROR;
  }

  const uint8_t* source = data;
  W25Q64_StatusTypeDef status = W25Q64_STATUS_OK;
  while ((length != 0U) && (status == W25Q64_STATUS_OK)) {
    size_t chunk = W25Q64_PAGE_SIZE - (address % W25Q64_PAGE_SIZE);
    if (chunk > length)
      chunk = length;
    status = w25q64_WriteEnable();
    uint8_t command[4] = {
      W25Q64_COMMAND_PAGE_PROGRAM,
      (uint8_t)(address >> 16U),
      (uint8_t)(address >> 8U),
      (uint8_t)address,
    };
    if (status == W25Q64_STATUS_OK) {
      w25q64_Select();
      if ((Spi1_Transfer(command, NULL, sizeof(command)) != SPI_STATUS_OK)
          || (Spi1_Transfer(source, NULL, chunk) != SPI_STATUS_OK)) {
        status = W25Q64_STATUS_IO_ERROR;
      }
      w25q64_Deselect();
    }
    if (status == W25Q64_STATUS_OK)
      status = w25q64_WaitReady(W25Q64_PROGRAM_TIMEOUT_MS);
    address += chunk;
    source += chunk;
    length -= chunk;
  }

  (void)xSemaphoreGive(flashMutex);
  return status;
}

/**
  * @brief Assert the active-low PA4 Flash chip-select signal.
  */
static void w25q64_Select(void) {
  GPIO_PIN_RESET(GPIOA, W25Q64_CHIP_SELECT_PIN_MASK);
}

/**
  * @brief Deassert the active-low PA4 Flash chip-select signal.
  */
static void w25q64_Deselect(void) {
  GPIO_PIN_SET(GPIOA, W25Q64_CHIP_SELECT_PIN_MASK);
}

/**
  * @brief Execute one command and optional read phase under one selection.
  * @param command (const uint8_t*) Non-null command and address bytes.
  * @param commandLength (size_t) Nonzero command byte count.
  * @param receive (uint8_t*) Optional response destination.
  * @param receiveLength (size_t) Response byte count; zero skips the read phase.
  * @retval W25Q64_STATUS_OK Both SPI phases completed.
  * @retval W25Q64_STATUS_TIMEOUT A bounded SPI flag wait expired.
  * @retval W25Q64_STATUS_IO_ERROR Another SPI failure occurred.
  */
static W25Q64_StatusTypeDef w25q64_Command(
  const uint8_t* command,
  size_t commandLength,
  uint8_t* receive,
  size_t receiveLength
) {
  w25q64_Select();
  Spi_StatusTypeDef spiStatus = Spi1_Transfer(
    command,
    NULL,
    commandLength
  );
  if ((spiStatus == SPI_STATUS_OK) && (receiveLength != 0U))
    spiStatus = Spi1_Transfer(NULL, receive, receiveLength);
  w25q64_Deselect();
  if (spiStatus == SPI_STATUS_OK)
    return W25Q64_STATUS_OK;
  return spiStatus == SPI_STATUS_TIMEOUT
    ? W25Q64_STATUS_TIMEOUT
    : W25Q64_STATUS_IO_ERROR;
}

/**
  * @brief Set the Flash write-enable latch for the next modifying command.
  * @retval (W25Q64_StatusTypeDef) SPI command result.
  */
static W25Q64_StatusTypeDef w25q64_WriteEnable(void) {
  const uint8_t command = W25Q64_COMMAND_WRITE_ENABLE;
  return w25q64_Command(&command, 1U, NULL, 0U);
}

/**
  * @brief Poll status-register bit zero until the Flash becomes ready.
  * @param timeoutMs (uint32_t) Maximum number of one-millisecond polls.
  * @retval W25Q64_STATUS_OK The busy bit cleared.
  * @retval W25Q64_STATUS_IO_ERROR A status-register read failed.
  * @retval W25Q64_STATUS_TIMEOUT The busy bit remained set.
  * @note Uses an RTOS delay while the scheduler runs and a DWT delay during
  *       pre-scheduler initialization.
  */
static W25Q64_StatusTypeDef w25q64_WaitReady(uint32_t timeoutMs) {
  for (uint32_t elapsedMs = 0U; elapsedMs < timeoutMs; elapsedMs++) {
    const uint8_t command = W25Q64_COMMAND_STATUS;
    uint8_t status = 0U;
    if (w25q64_Command(&command, 1U, &status, 1U)
        != W25Q64_STATUS_OK) {
      return W25Q64_STATUS_IO_ERROR;
    }
    if ((status & W25Q64_STATUS_BUSY) == 0U)
      return W25Q64_STATUS_OK;
    if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING)
      vTaskDelay(pdMS_TO_TICKS(1U));
    else
      Common_DelayMicroseconds(1000U);
  }
  return W25Q64_STATUS_TIMEOUT;
}

/**
  * @brief Validate a nonempty range without unsigned wraparound.
  * @param address (uint32_t) First requested byte address.
  * @param length (size_t) Requested byte count.
  * @retval 1 The entire range lies inside the 8 MiB device.
  * @retval 0 The range is empty, too large, or extends beyond the device.
  */
static uint8_t w25q64_IsRangeValid(uint32_t address, size_t length) {
  return ((length != 0U)
      && (length <= W25Q64_CAPACITY_BYTES)
      && (address <= W25Q64_CAPACITY_BYTES - length))
    ? 1U
    : 0U;
}
