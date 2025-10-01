/**
  ******************************************************************************
  * @file           : common.c
  * @brief          : The C code file provides a collection of commonly used 
  *                   routines along with the printf() function for outputting
  *                   data.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
 
/* Includes ------------------------------------------------------------------*/
#include "common.h"

/* Global variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/**
  * @brief  The function is responsible for resetting all peripherals,
  *         initializing the Flash interface, Systick, clocks, and
  *         other peripherals on the STM32F767 microcontroller.
  * @retval None
  */
void SystemInit(void) {
  /* Vector Table Relocation in Internal FLASH */
  SCB->VTOR = FLASH_BASE | 0x00;

 /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();

  /* Enable the FLASH Adaptive Real-Time memory accelerator */
  SET_BIT(FLASH->ACR, FLASH_ACR_ARTEN);

  /* Enable the FLASH prefetch buffer */
  FLASH->ACR |= FLASH_ACR_PRFTEN;

  /* Set Interrupt Group Priority */
  NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

  /* Conficure SysTick */
  SET_BIT(SysTick->CTRL, (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk));
  SysTick->LOAD = 216000U - 1U;
  SysTick->VAL = 0;
  SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);

  /* SysTick interrupt configuration */
  NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(SysTick_IRQn);

  /* SysCfg */
  PREG_SET(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN_Pos);
  while (!(PREG_CHECK(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN_Pos)));
   
  /* PWR */
  PREG_SET(RCC->APB1ENR, RCC_APB1ENR_PWREN_Pos);
  while (!(PREG_CHECK(RCC->APB1ENR, RCC_APB1ENR_PWREN_Pos)));
  PREG_SET(RCC->APB1ENR, RCC_APB2ENR_SYSCFGEN_Pos);
  while (!(PREG_CHECK(RCC->APB1ENR, RCC_APB2ENR_SYSCFGEN_Pos)));

  /* FLASH_IRQn interrupt configuration */
  NVIC_SetPriority(FLASH_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(FLASH_IRQn);

  /* RCC_IRQn interrupt configuration */
  NVIC_SetPriority(RCC_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 0, 0));
  NVIC_EnableIRQ(RCC_IRQn);

  /* Set the FLASH Latency */
  MODIFY_REG(FLASH->ACR, FLASH_ACR_LATENCY, FLASH_ACR_LATENCY_7WS);
  if (READ_BIT(FLASH->ACR, FLASH_ACR_LATENCY) != FLASH_ACR_LATENCY_7WS) {
    Error_Handler();
  }

  /* Configure the main internal regulator output voltage */
  PREG_SET(PWR->CR1, PWR_CR1_VOS_Pos);
  while (!(PREG_CHECK(PWR->CR1, PWR_CR1_VOS_Pos)));
  /* Enable Over drive Mode */
  PREG_SET(PWR->CR1, PWR_CR1_ODEN_Pos);

  /* HSE enable and wait till it's ready */
  PREG_SET(RCC->CR, RCC_CR_HSEON_Pos);
  while(!(PREG_CHECK(RCC->CR, RCC_CR_HSERDY_Pos)));

  /* LSI enable and wait till it's ready */
  PREG_SET(RCC->CSR, RCC_CSR_LSION_Pos);
  while(!(PREG_CHECK(RCC->CSR, RCC_CSR_LSIRDY_Pos)));

  /* Backup registers enable access */
  PREG_SET(PWR->CR1, PWR_CR1_DBP_Pos);

  /* force backup domain reset */
  PREG_SET(RCC->BDCR, RCC_BDCR_BDRST_Pos);
  PREG_CLR(RCC->BDCR, RCC_BDCR_BDRST_Pos);

  /* Confugure LSE drive */
  MODIFY_REG(RCC->BDCR, RCC_BDCR_LSEDRV, 0U);

  /* LSE enable and wait till it's ready */
  PREG_SET(RCC->BDCR, RCC_BDCR_LSEON_Pos);
  while(!(PREG_CHECK(RCC->BDCR, RCC_BDCR_LSERDY_Pos)));

  /* RTC source is LSE */
  MODIFY_REG(RCC->BDCR, RCC_BDCR_RTCSEL, RCC_BDCR_RTCSEL_0);

  /* Enable RTC */
  PREG_SET(RCC->BDCR, RCC_BDCR_RTCEN_Pos);

  /* Configure PLL domain end prescaller */
  /* main clock*/
  MODIFY_REG(RCC->PLLCFGR, (RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP), ((RCC_PLLCFGR_PLLSRC_HSE | RCC_PLLCFGR_PLLM_2) | 216 << RCC_PLLCFGR_PLLN_Pos));
  /* USB clock*/
  MODIFY_REG(RCC->PLLCFGR, (RCC_PLLCFGR_PLLSRC | RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLQ), ((RCC_PLLCFGR_PLLSRC_HSE | RCC_PLLCFGR_PLLM_2) | 216 << RCC_PLLCFGR_PLLN_Pos | RCC_PLLCFGR_PLLQ_3));
  /* PLL enable and wait till it's ready */
  PREG_SET(RCC->CR, RCC_CR_PLLON_Pos);
  while (!(PREG_CHECK(RCC->CR, RCC_CR_PLLRDY_Pos)));

  /* AHB isn't divided */
  MODIFY_REG(RCC->CFGR, RCC_CFGR_HPRE, RCC_CFGR_HPRE_DIV1);

  /* APB1 divided by 4 */
  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE1, RCC_CFGR_PPRE1_DIV4);
  /* APB2 divided by 2 */
  MODIFY_REG(RCC->CFGR, RCC_CFGR_PPRE2, RCC_CFGR_PPRE2_DIV2);

  /* Enable PLL as sysclock and wait till it's ready */
  MODIFY_REG(RCC->CFGR, RCC_CFGR_SW, RCC_CFGR_SW_PLL);
  while(!(READ_BIT(RCC->CFGR, RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL));

  // MODIFY_REG(RCC->DCKCFGR2, (((RCC_DCKCFGR2_USART1SEL << 16U) | RCC_DCKCFGR2_USART1SEL_0) >> 16U), (((RCC_DCKCFGR2_USART1SEL << 16U) | RCC_DCKCFGR2_USART1SEL_0) & 0x0000FFFFU));

  /* Set timers prescaler */
  MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_TIMPRE_Msk, (0 << RCC_DCKCFGR1_TIMPRE_Pos));

  /* Enable PLL SAI Domain */
  RCC->PLLSAICFGR = (
      192 << RCC_PLLSAICFGR_PLLSAIN_Pos
    | 2 << RCC_PLLSAICFGR_PLLSAIP_Pos
    | 2 << RCC_PLLSAICFGR_PLLSAIQ_Pos
    | 2 << RCC_PLLSAICFGR_PLLSAIR_Pos
  );
  MODIFY_REG(RCC->DCKCFGR1, RCC_DCKCFGR1_PLLSAIDIVR_Msk, (0 << RCC_DCKCFGR1_PLLSAIDIVR_Pos));

  /* Enable PLL SAI wait till it's ready */
  PREG_SET(RCC->CR, RCC_CR_PLLSAION_Pos);
  while (!(PREG_CHECK(RCC->CR, RCC_CR_PLLSAIRDY_Pos)));


  SET_BIT(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Msk);


  SystemClocks.SystemCore       = 216000000U;
  SystemClocks.HCLK_Freq        = 216000000U;
  SystemClocks.PCLK1_Freq       = 54000000U;
  SystemClocks.PCLK1_Freq_Tim   = 108000000U;
  SystemClocks.PCLK2_Freq       = 108000000U;
  SystemClocks.PCLK2_Freq_Tim   = 216000000U;
  SystemClocks.PLLQ_Freq        = 48000000U;


  /*****************************************************************************************/
  /*****************************************************************************************/
  /*****************************************************************************************/
  /* Freeze some peripherals for debug purposes */
  #ifdef DEBUG
  SET_BIT(DBGMCU->APB1FZ, (
      DBGMCU_APB1_FZ_DBG_TIM7_STOP
    | DBGMCU_APB1_FZ_DBG_TIM6_STOP
    | DBGMCU_APB1_FZ_DBG_IWDG_STOP
    | DBGMCU_APB1_FZ_DBG_WWDG_STOP
    | DBGMCU_APB1_FZ_DBG_I2C2_SMBUS_TIMEOUT
  ));
  #endif

  /* Set TPUI output to ITM */
  // PREG_SET(CoreDebug->DEMCR, CoreDebug_DEMCR_TRCENA_Pos);
  // TPI->CSPSR = 0x1;
  // TPI->FFCR = 0x102;
  // TPI->SPPR = 0x2;
  // PREG_SET(DBGMCU->CR, DBGMCU_CR_TRACE_IOEN_Pos);
  // ITM->LAR = 0xc5acce55;
  // ITM->TCR = (ITM_TCR_ITMENA_Msk | ITM_TCR_SYNCENA_Msk | (1 << ITM_TCR_TraceBusID_Pos));
  // ITM->TER = 0x1;
  // ITM->TPR = 0x1;


  /*****************************************************************************************/
  // /* IWDG */
  // IWDG->KR = IWDG_KEY_ENABLE;
  // IWDG->KR = IWDG_KEY_WR_ACCESS_ENABLE;
  // IWDG->PR =  IWDG_PR_PR & (IWDG_PR_PR_2 | IWDG_PR_PR_0); /*!< Divided by 128 */
  // IWDG->RLR = IWDG_RLR_RL & 624;                          /*<! ~2.5sec.  */
  // while (!(PREG_CHECK(IWDG->SR, IWDG_SR_PVU_Pos)));
  // IWDG->KR = IWDG_KEY_RELOAD;

  /*****************************************************************************************/
  /* Peripheral clock */
  /* APB1 */
  SET_BIT(RCC->APB1ENR, (
      RCC_APB1ENR_TIM6EN
    | RCC_APB1ENR_USART3EN
    | RCC_APB1ENR_I2C2EN
  ));

  /* AHB1 */
  SET_BIT(RCC->AHB1ENR, ( 
      RCC_AHB1ENR_GPIOBEN
    | RCC_AHB1ENR_GPIOCEN
    | RCC_AHB1ENR_GPIODEN
    | RCC_AHB1ENR_GPIOFEN
  ));

  /* APB2 */
  // SET_BIT(RCC->APB2ENR, (
  // ));

}
