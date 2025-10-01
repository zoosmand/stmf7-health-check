/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
static uint32_t _ASREG_ = 0;

/* Global variables ----------------------------------------------------------*/
RCC_ClocksTypeDef SystemClocks;
__IO uint32_t SysTickCnt;

/* Peripheral initialization statuses ----------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {

  if (Init_LED()) FLAG_SET(_ASREG_, LED_BLUE_READY_flag);
  if (Init_ETH_LL()) FLAG_SET(_ASREG_, ETH_READY_flag);

  while (1) {
    __NOP();

    // LED_Blink(LED_RED_Port, LED_RED_Pin);
    // _delay_ms(1000);
    // printf("test\n");
    // _delay_us(1000);


  }
}

