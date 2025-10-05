/**
  ******************************************************************************
  * @file           : led.c
  * @brief          : This file contains the common defines for the GPIO
  *                   initialization functions. 
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 

  /* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* Global variables ---------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/




int Init_HeartBeat(GPIO_TypeDef* port, uint16_t pin) {
  /* Mode */
  MODIFY_REG(HEAR_BEAT_PORT->MODER, HEAR_BEAT_PIN_Mask, (_MODE_OUT << HEAR_BEAT_PIN_Pos * 2));
  /* Speed */
  // MODIFY_REG(LED_BLUE_Port->OSPEEDR, LED_BLUE_Pin_Mask, (_SPEED_L << LED_BLUE_Pin_Pos * 2));
  // /* Output type */
  // MODIFY_REG(LED_BLUE_Port->OTYPER, (_OTYPE_PP << LED_BLUE_Pin_Mask), (_OTYPE_PP << LED_BLUE_Pin_Pos));
  // /* Push mode */
//   MODIFY_REG(HEAR_BEAT_PORT->PUPDR, HEAR_BEAT_PIN_Mask, (_PUPD_PD << HEAR_BEAT_PIN_Pos * 2));

  return (0);
}
