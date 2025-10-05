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
  if (!Init_HeartBeat(HEAR_BEAT_PORT, HEAR_BEAT_PIN_Pos)) FLAG_SET(_PREG_, _PR_HEART_BEAT_LED);


  /* Run the Heartbeat Service */
  HeartBeatService();


  /* Start the scheduler. */
  vTaskStartScheduler();


  while (1) {
    __NOP();
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
