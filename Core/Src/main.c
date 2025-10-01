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

  if (Init_LED()) FLAG_SET(_ASREG_, LED_BLUE_flag);

  while (1) {
    __NOP();

    LED_Blink(LED_BLUE_Port, LED_BLUE_Pin);
    _delay_ms(1000);
  }
}

