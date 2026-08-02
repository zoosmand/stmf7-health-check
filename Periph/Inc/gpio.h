/**
  ******************************************************************************
  * @file           : gpio.h
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

#ifndef GPIO_H
#define GPIO_H

#include "common.h"

/**
  * @brief Configure one GPIO as a low-speed push-pull output.
  * @param port (GPIO_TypeDef*) Non-null GPIO peripheral.
  * @param pinPosition (uint32_t) Pin position from 0 through 15.
  */
void Gpio_InitOutput(GPIO_TypeDef* port, uint32_t pinPosition);

#endif /* GPIO_H */
