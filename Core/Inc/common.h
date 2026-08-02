/**
  ******************************************************************************
  * @file           : common.h
  * @brief          : Common register, GPIO, timing, and error interfaces.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 29.09.2025
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

#ifndef COMMON_H
#define COMMON_H

#include "stm32f7xx.h"

#include <stdint.h>

#define BIT_SET(registerValue, bitPosition) \
  SET_BIT((registerValue), (1UL << (bitPosition)))
#define BIT_CLEAR(registerValue, bitPosition) \
  CLEAR_BIT((registerValue), (1UL << (bitPosition)))
#define BIT_IS_SET(registerValue, bitPosition) \
  (READ_BIT((registerValue), (1UL << (bitPosition))) != 0UL)

#define GPIO_PIN_SET(port, pinMask) SET_BIT((port)->BSRR, (pinMask))
#define GPIO_PIN_RESET(port, pinMask) \
  SET_BIT((port)->BSRR, ((uint32_t)(pinMask) << 16U))
#define GPIO_PIN_IS_HIGH(port, pinMask) \
  (READ_BIT((port)->IDR, (pinMask)) != 0UL)

#define GPIO_MODE_INPUT     0x0UL
#define GPIO_MODE_OUTPUT    0x1UL
#define GPIO_MODE_ALTERNATE 0x2UL
#define GPIO_MODE_ANALOG    0x3UL

#define GPIO_OUTPUT_PUSH_PULL  0x0UL
#define GPIO_OUTPUT_OPEN_DRAIN 0x1UL

#define GPIO_SPEED_LOW       0x0UL
#define GPIO_SPEED_MEDIUM    0x1UL
#define GPIO_SPEED_HIGH      0x2UL
#define GPIO_SPEED_VERY_HIGH 0x3UL

#define GPIO_NO_PULL   0x0UL
#define GPIO_PULL_UP   0x1UL
#define GPIO_PULL_DOWN 0x2UL

/**
  * @brief Frequencies of the clocks configured by SystemInit().
  * @param systemCoreHz (uint32_t) Cortex-M7 core frequency in hertz.
  * @param hclkHz (uint32_t) AHB bus frequency in hertz.
  * @param pclk1Hz (uint32_t) APB1 peripheral frequency in hertz.
  * @param pclk1TimerHz (uint32_t) APB1 timer frequency in hertz.
  * @param pclk2Hz (uint32_t) APB2 peripheral frequency in hertz.
  * @param pclk2TimerHz (uint32_t) APB2 timer frequency in hertz.
  * @param pll48Hz (uint32_t) PLL 48 MHz domain frequency in hertz.
  */
typedef struct {
  uint32_t systemCoreHz;
  uint32_t hclkHz;
  uint32_t pclk1Hz;
  uint32_t pclk1TimerHz;
  uint32_t pclk2Hz;
  uint32_t pclk2TimerHz;
  uint32_t pll48Hz;
} SystemClocks_TypeDef;

extern SystemClocks_TypeDef systemClocks;

/**
  * @brief Stop execution after an unrecoverable system failure.
  */
void System_ErrorHandler(void) __attribute__((noreturn));

/**
  * @brief Delay execution using the Cortex-M7 cycle counter.
  * @param microseconds (uint32_t) Delay duration in microseconds.
  * @note This busy wait is intended only for short pre-scheduler hardware
  *       timing. FreeRTOS tasks must use an RTOS delay.
  */
void Common_DelayMicroseconds(uint32_t microseconds);

#endif /* COMMON_H */
