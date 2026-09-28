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

#include "FreeRTOS.h"
#include "semphr.h"

#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>

static void common_WriteCharacter(uint8_t character);
static StaticSemaphore_t outputMutexControlBlock;
static SemaphoreHandle_t outputMutex;

ErrorStatus Common_InitOutput(void) {
  if (outputMutex == NULL)
    outputMutex = xSemaphoreCreateMutexStatic(&outputMutexControlBlock);
  return outputMutex != NULL ? SUCCESS : ERROR;
}

int Common_Printf(const char* format, ...) {
  if (format == NULL)
    return -1;
  if ((outputMutex != NULL)
      && (xSemaphoreTake(outputMutex, portMAX_DELAY) != pdTRUE)) {
    return -1;
  }
  va_list arguments;
  va_start(arguments, format);
  int result = vprintf(format, arguments);
  va_end(arguments);
  if (outputMutex != NULL)
    (void)xSemaphoreGive(outputMutex);
  return result;
}

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
  Common_Printf("Assertion failed: %s:%lu\n", file, (unsigned long)line);
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

#ifdef USART_OUT
  while (READ_BIT(USART_OUT->ISR, USART_ISR_TXE) == 0U) {
    __NOP();
  }
  USART_OUT->TDR = character;
#endif
}
