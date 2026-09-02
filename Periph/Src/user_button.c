/**
  ******************************************************************************
  * @file           : user_button.c
  * @brief          : NUCLEO-F767ZI B1 user-button access.
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

#include "user_button.h"

#include "common.h"

#define USER_BUTTON_PIN_POSITION 13U
#define USER_BUTTON_PIN_MASK     (1UL << USER_BUTTON_PIN_POSITION)

void UserButton_Init(void) {
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  (void)RCC->AHB1ENR;
  GPIOC->MODER &= ~(0x3UL << (USER_BUTTON_PIN_POSITION * 2U));
  GPIOC->PUPDR &= ~(0x3UL << (USER_BUTTON_PIN_POSITION * 2U));
}

uint8_t UserButton_IsPressed(void) {
  return GPIO_PIN_IS_HIGH(GPIOC, USER_BUTTON_PIN_MASK) ? 1U : 0U;
}
