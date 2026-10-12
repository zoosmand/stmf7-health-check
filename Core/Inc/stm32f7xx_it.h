/**
  ******************************************************************************
  * @file           : stm32f7xx_it.h
  * @brief          : Cortex-M7 exception-handler declarations.
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

#ifndef STM32F7XX_IT_H
#define STM32F7XX_IT_H

/** @brief Handle a non-maskable interrupt by stopping in a diagnostic loop. */
void NMI_Handler(void);
/** @brief Handle a hard fault by stopping in a diagnostic loop. */
void HardFault_Handler(void);
/** @brief Handle a memory-management fault in a diagnostic loop. */
void MemManage_Handler(void);
/** @brief Handle a bus fault by stopping in a diagnostic loop. */
void BusFault_Handler(void);
/** @brief Handle a usage fault by stopping in a diagnostic loop. */
void UsageFault_Handler(void);
/** @brief Handle a debug-monitor exception. */
void DebugMon_Handler(void);

#endif /* STM32F7XX_IT_H */
