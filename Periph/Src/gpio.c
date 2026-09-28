/**
  ******************************************************************************
  * @file           : gpio.c
  * @brief          : Project-owned GPIO configuration helpers.
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

#include "gpio.h"

void Gpio_InitOutput(GPIO_TypeDef* port, uint32_t pinPosition) {
  uint32_t modeShift = pinPosition * 2U;
  uint32_t modeMask = 0x3UL << modeShift;
  uint32_t pinMask = 1UL << pinPosition;

  MODIFY_REG(port->MODER, modeMask, GPIO_MODE_OUTPUT << modeShift);
  MODIFY_REG(port->OSPEEDR, modeMask, GPIO_SPEED_LOW << modeShift);
  CLEAR_BIT(port->OTYPER, pinMask);
  CLEAR_BIT(port->PUPDR, modeMask);
}
