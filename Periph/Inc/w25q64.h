/**
  ******************************************************************************
  * @file           : w25q64.h
  * @brief          : Bounded W25Q64 NOR Flash access.
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

#ifndef W25Q64_H
#define W25Q64_H

#include <stddef.h>
#include <stdint.h>

#define W25Q64_CAPACITY_BYTES (8U * 1024U * 1024U)
#define W25Q64_SECTOR_SIZE     4096U
#define W25Q64_PAGE_SIZE       256U

/**
  * @brief Result of a W25Q64 operation.
  */
typedef enum {
  W25Q64_STATUS_OK = 0,            /**< Operation completed successfully. */
  W25Q64_STATUS_INVALID_ARGUMENT,  /**< Address, length, or pointer invalid. */
  W25Q64_STATUS_IO_ERROR,          /**< Mutex or SPI transaction failed. */
  W25Q64_STATUS_TIMEOUT,           /**< SPI or Flash busy wait timed out. */
  W25Q64_STATUS_UNAVAILABLE,       /**< JEDEC identity did not match. */
} W25Q64_StatusTypeDef;

/**
  * @brief Create the Flash mutex and verify the JEDEC identity.
  * @retval W25Q64_STATUS_OK Mutex exists and JEDEC identity is EF 40 17.
  * @retval W25Q64_STATUS_IO_ERROR Static mutex creation failed.
  * @retval W25Q64_STATUS_UNAVAILABLE The expected identity was not received.
  * @note This read-only probe does not erase or program the device.
  */
W25Q64_StatusTypeDef W25Q64_Init(void);

/**
  * @brief Verify that the expected W25Q64 responds on SPI1.
  * @retval 1 The JEDEC identity is EF 40 17.
  * @retval 0 The driver is uninitialized, locking failed, the SPI transaction
  *         failed, or the identity did not match.
  * @note This blocking function must be called from task context after init.
  */
uint8_t W25Q64_IsAvailable(void);

/**
  * @brief Read bytes from the Flash address space.
  * @param address (uint32_t) First 24-bit Flash address to read.
  * @param data (void*) Non-null destination with space for `length` bytes.
  * @param length (size_t) Nonzero byte count contained within the 8 MiB device.
  * @retval W25Q64_STATUS_OK The requested bytes were read.
  * @retval W25Q64_STATUS_INVALID_ARGUMENT The range or destination is invalid.
  * @retval W25Q64_STATUS_IO_ERROR The driver is uninitialized or locking failed.
  * @retval W25Q64_STATUS_TIMEOUT The SPI transaction timed out.
  * @note This blocking function serializes access with the Flash mutex.
  */
W25Q64_StatusTypeDef W25Q64_Read(
  uint32_t address,
  void* data,
  size_t length
);

/**
  * @brief Erase one aligned 4 KiB sector.
  * @param address (uint32_t) Start of a sector within the 8 MiB address space.
  * @retval W25Q64_STATUS_OK The erase completed and the busy flag cleared.
  * @retval W25Q64_STATUS_INVALID_ARGUMENT The address is out of range or not
  *         aligned to W25Q64_SECTOR_SIZE.
  * @retval W25Q64_STATUS_IO_ERROR Locking or an SPI transaction failed.
  * @retval W25Q64_STATUS_TIMEOUT SPI or the Flash busy state timed out.
  * @note This blocking operation may take up to two seconds.
  */
W25Q64_StatusTypeDef W25Q64_EraseSector(uint32_t address);

/**
  * @brief Erase a contiguous, sector-aligned range under one Flash lock.
  * @param address (uint32_t) First aligned sector address.
  * @param length (size_t) Nonzero erase length, in whole sectors.
  * @retval (W25Q64_StatusTypeDef) Result of all sector erase operations.
  * @note Sectors are erased from the highest address to the lowest address.
  */
W25Q64_StatusTypeDef W25Q64_EraseRange(uint32_t address, size_t length);

/**
  * @brief Program bytes while respecting 256-byte page boundaries.
  * @param address (uint32_t) First 24-bit Flash address to program.
  * @param data (const void*) Non-null source containing `length` bytes.
  * @param length (size_t) Nonzero byte count contained within the 8 MiB device.
  * @retval W25Q64_STATUS_OK Every page chunk completed successfully.
  * @retval W25Q64_STATUS_INVALID_ARGUMENT The source or range is invalid.
  * @retval W25Q64_STATUS_IO_ERROR Locking or an SPI transaction failed.
  * @retval W25Q64_STATUS_TIMEOUT SPI or a page-program busy wait timed out.
  * @note The destination must already be erased. This function does not verify
  *       programmed contents.
  */
W25Q64_StatusTypeDef W25Q64_Program(
  uint32_t address,
  const void* data,
  size_t length
);

#endif /* W25Q64_H */
