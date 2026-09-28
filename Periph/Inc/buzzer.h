/**
  ******************************************************************************
  * @file           : buzzer.h
  * @brief          : TIM2/PA3 passive-buzzer PWM tone driver.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 03.08.2026
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

#ifndef BUZZER_H
#define BUZZER_H

/**
  * @brief Configure TIM2 channel 4 for a fixed audible tone and leave the
  *        buzzer output driven to a safe, silent state.
  * @note PA3 is held as a low GPIO output until Buzzer_On() runs, rather than
  *       left in its alternate-function mode, because TIM2 has no output
  *       idle-state control (that feature exists only on advanced-control
  *       timers) and the pin state while the channel is disabled is otherwise
  *       unspecified.
  */
void Buzzer_Init(void);

/**
  * @brief Switch PA3 to TIM2 channel 4 and start the tone from a fresh cycle.
  * @note Blocking-free; the caller is responsible for timing the tone
  *       duration and calling Buzzer_Off() afterward.
  */
void Buzzer_On(void);

/**
  * @brief Stop TIM2 channel 4 and return PA3 to a low GPIO output.
  */
void Buzzer_Off(void);

#endif /* BUZZER_H */
