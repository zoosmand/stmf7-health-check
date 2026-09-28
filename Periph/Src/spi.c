/**
  ******************************************************************************
  * @file           : spi.c
  * @brief          : CMSIS SPI1 initialization and bounded byte transfers.
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

#include "spi.h"

#include "common.h"
#include "gpio.h"

#define SPI1_PIN_ALTERNATE_FUNCTION 5U
#define SPI1_PIN_SCK                3U
#define SPI1_PIN_MISO               4U
#define SPI1_PIN_MOSI               5U
#define SPI1_PIN_CHIP_SELECT        4U
#define SPI1_WAIT_LOOPS             1000000U

static void spi1_ConfigureAlternatePin(uint32_t pinPosition);
static Spi_StatusTypeDef spi1_WaitForFlag(uint32_t mask, uint8_t mustBeSet);

/**
  * @brief Initialize the board's SPI1 bus and W25Q64 chip-select output.
  */
void Spi1_Init(void) {
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN);
  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SPI1EN);
  (void)RCC->APB2ENR;

  GPIO_PIN_SET(GPIOA, 1UL << SPI1_PIN_CHIP_SELECT);
  Gpio_InitOutput(GPIOA, SPI1_PIN_CHIP_SELECT);
  spi1_ConfigureAlternatePin(SPI1_PIN_SCK);
  spi1_ConfigureAlternatePin(SPI1_PIN_MISO);
  spi1_ConfigureAlternatePin(SPI1_PIN_MOSI);

  CLEAR_BIT(SPI1->CR1, SPI_CR1_SPE);
  SPI1->CR1 = SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_BR_0;
  SPI1->CR2 = SPI_CR2_FRXTH | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0;
  SET_BIT(SPI1->CR1, SPI_CR1_SPE);
}

/**
  * @brief Perform a bounded, polling-based full-duplex SPI1 transfer.
  * @param transmitData (const uint8_t*) Optional byte source or NULL for 0xFF.
  * @param receiveData (uint8_t*) Optional byte destination or NULL to discard.
  * @param length (size_t) Nonzero number of bytes to exchange.
  * @retval (Spi_StatusTypeDef) Detailed transfer result.
  */
Spi_StatusTypeDef Spi1_Transfer(
  const uint8_t* transmitData,
  uint8_t* receiveData,
  size_t length
) {
  if (length == 0U)
    return SPI_STATUS_INVALID_ARGUMENT;

  for (size_t index = 0U; index < length; index++) {
    if (spi1_WaitForFlag(SPI_SR_TXE, 1U) != SPI_STATUS_OK)
      return SPI_STATUS_TIMEOUT;
    *(__IO uint8_t*)&SPI1->DR = transmitData != NULL
      ? transmitData[index]
      : 0xFFU;
    if (spi1_WaitForFlag(SPI_SR_RXNE, 1U) != SPI_STATUS_OK)
      return SPI_STATUS_TIMEOUT;
    uint8_t received = *(__IO uint8_t*)&SPI1->DR;
    if (receiveData != NULL)
      receiveData[index] = received;
  }

  return spi1_WaitForFlag(SPI_SR_BSY, 0U);
}

/**
  * @brief Configure one GPIOB pin as an SPI1 AF5 signal.
  * @param pinPosition (uint32_t) GPIOB pin position from 0 through 15.
  * @note The caller enables the GPIOB clock before calling this helper.
  */
static void spi1_ConfigureAlternatePin(uint32_t pinPosition) {
  uint32_t modeShift = pinPosition * 2U;
  uint32_t alternateShift = (pinPosition % 8U) * 4U;
  MODIFY_REG(
    GPIOB->MODER,
    0x3UL << modeShift,
    GPIO_MODE_ALTERNATE << modeShift
  );
  MODIFY_REG(
    GPIOB->OSPEEDR,
    0x3UL << modeShift,
    GPIO_SPEED_VERY_HIGH << modeShift
  );
  CLEAR_BIT(GPIOB->OTYPER, 1UL << pinPosition);
  CLEAR_BIT(GPIOB->PUPDR, 0x3UL << modeShift);
  MODIFY_REG(
    GPIOB->AFR[pinPosition / 8U],
    0xFUL << alternateShift,
    SPI1_PIN_ALTERNATE_FUNCTION << alternateShift
  );
}

/**
  * @brief Poll one SPI1 status flag until it reaches the requested state.
  * @param mask (uint32_t) Single SPI_SR flag mask to observe.
  * @param mustBeSet (uint8_t) Nonzero to wait for set; zero to wait for clear.
  * @retval SPI_STATUS_OK The requested state was observed.
  * @retval SPI_STATUS_TIMEOUT The polling budget expired first.
  */
static Spi_StatusTypeDef spi1_WaitForFlag(uint32_t mask, uint8_t mustBeSet) {
  uint32_t remaining = SPI1_WAIT_LOOPS;
  while (((READ_BIT(SPI1->SR, mask) != 0U) ? 1U : 0U) != mustBeSet) {
    if (--remaining == 0U)
      return SPI_STATUS_TIMEOUT;
  }
  return SPI_STATUS_OK;
}
