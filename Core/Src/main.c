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

/* Global variables ----------------------------------------------------------*/
__IO uint32_t _PREG_ = 0;

/* Peripheral initialization statuses ----------------------------------------*/

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {

  /* Initialization of necessary peripherals */
  if (!Init_HeartBeat(HEAR_BEAT_PORT, HEAR_BEAT_PIN_Pos)) FLAG_SET(_PREG_, HEART_BEAT_LED_READY_Flag);
   if (Init_ETH_LL()) FLAG_SET(_PREG_, ETH_READY_Flag);


  /* Run the Heartbeat Service */
  HeartBeatService();


  /* Start the scheduler. */
  vTaskStartScheduler();


  while (1) {
    __NOP();

    // LED_Blink(LED_RED_Port, LED_RED_Pin);
    // _delay_ms(1000);
    // printf("test\n");
    // _delay_us(1000);


  }
}



#if (configCHECK_FOR_STACK_OVERFLOW > 0)

    void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
        /* Check pcTaskName for the name of the offending task,
         * or pxCurrentTCB if pcTaskName has itself been corrupted. */
        (void) xTask;
        (void) pcTaskName;
    }

#endif /* #if (configCHECK_FOR_STACK_OVERFLOW > 0) */
