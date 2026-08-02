/**
  ******************************************************************************
  * @file           : common.c
  * @brief          : Common timing, error, and standard-output routines.
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

#include "common.h"

#include <stddef.h>
#include <stdio.h>

static void common_WriteCharacter(uint8_t character);
static void common_WriteItm(uint8_t character, uint32_t channel);

void System_ErrorHandler(void) {
  __disable_irq();
  for (;;) {
    __NOP();
  }
}

void Common_DelayMicroseconds(uint32_t microseconds) {
  if (microseconds == 0U)
    return;

  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->LAR = 0xC5ACCE55UL;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  uint32_t cycles = microseconds * (systemClocks.systemCoreHz / 1000000U);
  while (DWT->CYCCNT < cycles) {
    __NOP();
  }
}

int _write(int file, char* data, int length) {
  (void)file;
  if ((data == NULL) || (length < 0))
    return -1;

  for (int index = 0; index < length; index++)
    common_WriteCharacter((uint8_t)data[index]);
  return length;
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t* file, uint32_t line) {
  printf("Assertion failed: %s:%lu\n", file, (unsigned long)line);
  System_ErrorHandler();
}
#endif

/**
  * @brief Write one character to each configured diagnostic destination.
  * @param character (uint8_t) Character to transmit.
  */
static void common_WriteCharacter(uint8_t character) {
  if (character == (uint8_t)'\n')
    common_WriteCharacter((uint8_t)'\r');

#ifdef ITM_OUT
  common_WriteItm(character, ITM_OUT);
#endif

#ifdef USART_OUT
  while (READ_BIT(USART_OUT->ISR, USART_ISR_TXE) == 0U) {
    __NOP();
  }
  USART_OUT->TDR = character;
#endif
}

/**
  * @brief Write one character to an enabled ITM stimulus port.
  * @param character (uint8_t) Character to transmit.
  * @param channel (uint32_t) ITM stimulus channel from 0 through 31.
  */
static void common_WriteItm(uint8_t character, uint32_t channel) {
  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) == 0U)
      || ((ITM->TER & (1UL << channel)) == 0U)) {
    return;
  }

  while (ITM->PORT[channel].u32 == 0U) {
    __NOP();
  }
  ITM->PORT[channel].u8 = character;
}
