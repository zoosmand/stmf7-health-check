/**
  ******************************************************************************
  * @file           : system.c
  * @brief          : STM32F767 clock, memory, and peripheral-clock setup.
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

#define SYSTEM_CLOCK_TIMEOUT_LOOPS 1000000U
#define SYSTEM_LSE_TIMEOUT_LOOPS   100000000U
#define ETHERNET_MEMORY_BASE       0x20078000UL
#define MPU_REGION_SIZE_32_KB      14UL

SystemClocks_TypeDef systemClocks = {
  .systemCoreHz = 216000000U,
  .hclkHz = 216000000U,
  .pclk1Hz = 54000000U,
  .pclk1TimerHz = 108000000U,
  .pclk2Hz = 108000000U,
  .pclk2TimerHz = 216000000U,
  .pll48Hz = 48000000U,
};

uint32_t SystemCoreClock = 216000000U;

static void system_ConfigureEthernetMemory(void);
static void system_ConfigureRtcClock(void);
static void system_WaitForSet(
  volatile uint32_t* registerAddress,
  uint32_t mask,
  uint32_t timeoutLoops
);

void SystemInit(void) {
  SCB->VTOR = FLASH_BASE;
  system_ConfigureEthernetMemory();
  SCB_EnableICache();
  SCB_EnableDCache();

  SET_BIT(FLASH->ACR, FLASH_ACR_ARTEN | FLASH_ACR_PRFTEN);
  NVIC_SetPriorityGrouping(3U);

  SET_BIT(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
  (void)RCC->APB1ENR;

  MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_7WS);
  if (READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_7WS)
    System_ErrorHandler();

  MODIFY_REG(PWR->CR1, PWR_CR1_VOS, PWR_CR1_VOS);
  SET_BIT(PWR->CR1, PWR_CR1_ODEN);
  system_WaitForSet(
    &PWR->CSR1,
    PWR_CSR1_ODRDY,
    SYSTEM_CLOCK_TIMEOUT_LOOPS
  );
  SET_BIT(PWR->CR1, PWR_CR1_ODSWEN);
  system_WaitForSet(
    &PWR->CSR1,
    PWR_CSR1_ODSWRDY,
    SYSTEM_CLOCK_TIMEOUT_LOOPS
  );

  SET_BIT(RCC->CR, RCC_CR_HSEON);
  system_WaitForSet(&RCC->CR, RCC_CR_HSERDY, SYSTEM_CLOCK_TIMEOUT_LOOPS);
  SET_BIT(RCC->CSR, RCC_CSR_LSION);
  system_WaitForSet(&RCC->CSR, RCC_CSR_LSIRDY, SYSTEM_CLOCK_TIMEOUT_LOOPS);
  system_ConfigureRtcClock();

  RCC->PLLCFGR = (4UL << RCC_PLLCFGR_PLLM_Pos)
    | (216UL << RCC_PLLCFGR_PLLN_Pos)
    | (0UL << RCC_PLLCFGR_PLLP_Pos)
    | RCC_PLLCFGR_PLLSRC_HSE
    | (9UL << RCC_PLLCFGR_PLLQ_Pos);
  SET_BIT(RCC->CR, RCC_CR_PLLON);
  system_WaitForSet(&RCC->CR, RCC_CR_PLLRDY, SYSTEM_CLOCK_TIMEOUT_LOOPS);

  MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV1);
  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV4);
  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV2);
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
  system_WaitForSet(
    &RCC->CFGR,
    RCC_CFGR_SWS_PLL,
    SYSTEM_CLOCK_TIMEOUT_LOOPS
  );

  MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_TIMPRE, 0U);

#ifdef DEBUG
  SET_BIT(
    DBGMCU->APB1FZ,
    DBGMCU_APB1_FZ_DBG_IWDG_STOP | DBGMCU_APB1_FZ_DBG_WWDG_STOP
  );
#endif

  SET_BIT(
    RCC->AHB1ENR,
    RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN
      | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIOGEN
      | RCC_AHB1ENR_ETHMACEN | RCC_AHB1ENR_ETHMACTXEN
      | RCC_AHB1ENR_ETHMACRXEN
  );
  (void)RCC->AHB1ENR;
}

/**
  * @brief Configure the dedicated 32 KiB Ethernet SRAM region as non-cacheable.
  */
static void system_ConfigureEthernetMemory(void) {
  __DMB();
  MPU->CTRL = 0U;
  MPU->RNR = 0U;
  MPU->RBAR = ETHERNET_MEMORY_BASE;
  MPU->RASR = MPU_RASR_XN_Msk
    | (3UL << MPU_RASR_AP_Pos)
    | MPU_RASR_S_Msk
    | (MPU_REGION_SIZE_32_KB << MPU_RASR_SIZE_Pos)
    | MPU_RASR_ENABLE_Msk;
  MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
  __DSB();
  __ISB();
}

/**
  * @brief Preserve a valid LSE-backed RTC domain or initialize it once.
  */
static void system_ConfigureRtcClock(void) {
  SET_BIT(PWR->CR1, PWR_CR1_DBP);
  system_WaitForSet(&PWR->CR1, PWR_CR1_DBP, SYSTEM_CLOCK_TIMEOUT_LOOPS);

  if ((READ_BIT(RCC->BDCR, RCC_BDCR_RTCSEL) != RCC_BDCR_RTCSEL_0)
      || (READ_BIT(RCC->BDCR, RCC_BDCR_LSERDY) == 0U)) {
    SET_BIT(RCC->BDCR, RCC_BDCR_BDRST);
    CLEAR_BIT(RCC->BDCR, RCC_BDCR_BDRST);
    MODIFY_REG(RCC->BDCR, RCC_BDCR_LSEDRV, 0U);
    SET_BIT(RCC->BDCR, RCC_BDCR_LSEON);
    system_WaitForSet(&RCC->BDCR, RCC_BDCR_LSERDY, SYSTEM_LSE_TIMEOUT_LOOPS);
    MODIFY_REG(RCC->BDCR, RCC_BDCR_RTCSEL, RCC_BDCR_RTCSEL_0);
  }
  SET_BIT(RCC->BDCR, RCC_BDCR_RTCEN);
}

/**
  * @brief Wait for every requested register bit with a bounded startup timeout.
  * @param registerAddress (volatile uint32_t*) Peripheral register address.
  * @param mask (uint32_t) Bits that must become set.
  * @param timeoutLoops (uint32_t) Maximum polling iterations.
  */
static void system_WaitForSet(
  volatile uint32_t* registerAddress,
  uint32_t mask,
  uint32_t timeoutLoops
) {
  uint32_t remaining = timeoutLoops;
  while ((READ_REG(*registerAddress) & mask) != mask) {
    if (--remaining == 0U)
      System_ErrorHandler();
  }
}
