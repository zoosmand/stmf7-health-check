/**
  ******************************************************************************
  * @file           : spi.h
  * @brief          : CMSIS SPI1 interface for board peripherals.
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

#ifndef SPI_H
#define SPI_H

#include <stddef.h>
#include <stdint.h>

/**
  * @brief Result of a synchronous SPI operation.
  */
typedef enum {
  SPI_STATUS_OK = 0,             /**< Transfer completed successfully. */
  SPI_STATUS_INVALID_ARGUMENT,   /**< Transfer length was zero. */
  SPI_STATUS_TIMEOUT,            /**< A required hardware flag timed out. */
} Spi_StatusTypeDef;

/**
  * @brief Configure SPI1 mode 0 and its W25Q64 GPIO assignment.
  * @note PB3/PB4/PB5 use AF5. PA4 is initialized high as software NSS.
  * @note SPI1 uses an APB2/4 clock of 27 MHz, 8-bit frames, MSB first, and
  *       software slave management. Call once before any transfer.
  */
void Spi1_Init(void);

/**
  * @brief Exchange bytes synchronously over SPI1.
  * @param transmitData (const uint8_t*) Optional source containing at least
  *        `length` bytes. NULL transmits 0xFF dummy bytes.
  * @param receiveData (uint8_t*) Optional destination with space for at least
  *        `length` bytes. NULL discards simultaneously received bytes.
  * @param length (size_t) Number of full-duplex byte exchanges; must be
  *        greater than zero.
  * @retval SPI_STATUS_OK Every byte was exchanged and SPI1 became idle.
  * @retval SPI_STATUS_INVALID_ARGUMENT `length` was zero.
  * @retval SPI_STATUS_TIMEOUT TXE, RXNE, or BSY did not reach the required
  *         state within its bounded polling interval.
  * @note The caller owns chip-select timing. This function is blocking and
  *       must not be called from an interrupt handler.
  */
Spi_StatusTypeDef Spi1_Transfer(
  const uint8_t* transmitData,
  uint8_t* receiveData,
  size_t length
);

#endif /* SPI_H */
