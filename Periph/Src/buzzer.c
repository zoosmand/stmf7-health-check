/**
  ******************************************************************************
  * @file           : buzzer.c
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

#include "buzzer.h"

#include "common.h"
#include "gpio.h"

#define BUZZER_PIN_POSITION         3U
#define BUZZER_ALTERNATE_FUNCTION   1U
#define BUZZER_TIMER_TICK_HZ        1000000U
#define BUZZER_TONE_HZ              2500U

static void buzzer_ConfigureAlternatePin(void);

/**
  * @brief Configure TIM2 channel 4 for a fixed audible tone and leave the
  *        buzzer output driven to a safe, silent state.
  */
void Buzzer_Init(void) {
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN);
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
  (void)RCC->APB1ENR;

  Gpio_InitOutput(GPIOA, BUZZER_PIN_POSITION);
  GPIO_PIN_RESET(GPIOA, 1UL << BUZZER_PIN_POSITION);

  uint32_t prescaler =
    (systemClocks.pclk1TimerHz / BUZZER_TIMER_TICK_HZ) - 1U;
  uint32_t period = (BUZZER_TIMER_TICK_HZ / BUZZER_TONE_HZ) - 1U;

  TIM2->CR1 = 0U;
  TIM2->CCER = 0U;
  TIM2->PSC = prescaler;
  TIM2->ARR = period;
  TIM2->CCR4 = (period + 1U) / 2U;
  MODIFY_REG(
    TIM2->CCMR2,
    TIM_CCMR2_OC4M | TIM_CCMR2_OC4PE,
    TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE
  );
  SET_BIT(TIM2->CR1, TIM_CR1_ARPE);
  SET_BIT(TIM2->EGR, TIM_EGR_UG);
}

/**
  * @brief Switch PA3 to TIM2 channel 4 and start the tone from a fresh cycle.
  * @note TIM2 is started before PA3 leaves GPIO mode, so the pin moves
  *       directly from a driven-low GPIO output to a timer output that is
  *       already active, without a window where PA3 is in alternate-function
  *       mode but not yet driven by a running timer.
  */
void Buzzer_On(void) {
  SET_BIT(TIM2->EGR, TIM_EGR_UG);
  SET_BIT(TIM2->CCER, TIM_CCER_CC4E);
  SET_BIT(TIM2->CR1, TIM_CR1_CEN);
  buzzer_ConfigureAlternatePin();
}

/**
  * @brief Stop TIM2 channel 4 and return PA3 to a low GPIO output.
  * @note PA3 returns to GPIO mode before TIM2 stops, mirroring Buzzer_On()'s
  *       ordering so the pin is never left in alternate-function mode with no
  *       active timer output driving it.
  */
void Buzzer_Off(void) {
  Gpio_InitOutput(GPIOA, BUZZER_PIN_POSITION);
  GPIO_PIN_RESET(GPIOA, 1UL << BUZZER_PIN_POSITION);
  CLEAR_BIT(TIM2->CR1, TIM_CR1_CEN);
  CLEAR_BIT(TIM2->CCER, TIM_CCER_CC4E);
}

/**
  * @brief Configure PA3 as the TIM2 channel 4 alternate-function signal.
  */
static void buzzer_ConfigureAlternatePin(void) {
  uint32_t modeShift = BUZZER_PIN_POSITION * 2U;
  uint32_t alternateShift = BUZZER_PIN_POSITION * 4U;
  MODIFY_REG(
    GPIOA->MODER,
    0x3UL << modeShift,
    GPIO_MODE_ALTERNATE << modeShift
  );
  MODIFY_REG(
    GPIOA->OSPEEDR,
    0x3UL << modeShift,
    GPIO_SPEED_VERY_HIGH << modeShift
  );
  CLEAR_BIT(GPIOA->OTYPER, 1UL << BUZZER_PIN_POSITION);
  CLEAR_BIT(GPIOA->PUPDR, 0x3UL << modeShift);
  MODIFY_REG(
    GPIOA->AFR[0],
    0xFUL << alternateShift,
    BUZZER_ALTERNATE_FUNCTION << alternateShift
  );
}
