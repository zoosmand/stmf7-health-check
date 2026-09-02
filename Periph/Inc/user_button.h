/**
  ******************************************************************************
  * @file           : user_button.h
  * @brief          : NUCLEO-F767ZI B1 user-button interface.
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

#ifndef USER_BUTTON_H
#define USER_BUTTON_H

#include <stdint.h>

/** @brief Configure the default B1 PC13 connection as an input. */
void UserButton_Init(void);

/** @retval (uint8_t) One while B1 is pressed; otherwise zero. */
uint8_t UserButton_IsPressed(void);

#endif /* USER_BUTTON_H */
