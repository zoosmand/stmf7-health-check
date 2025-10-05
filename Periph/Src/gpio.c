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




int Init_HeartBeat(GPIO_TypeDef* port, uint16_t pinPos) {
  /* Mode */
  MODIFY_REG(port->MODER, (0x3 << (pinPos * 2)), (_MODE_OUT << (pinPos * 2)));
  // /* Speed */
  // MODIFY_REG(port->OSPEEDR, (0x3 << (pinPos * 2)), (_SPEED_L << pinPos * 2));
  // /* Output type */
  // MODIFY_REG(port->OTYPER, (_OTYPE_PP << (0x3 << (pinPos * 2))), (_OTYPE_PP << pinPos));
  // /* Push mode */
  // MODIFY_REG(port->PUPDR, (0x3 << (pinPos * 2)), (_PUPD_NO << HEAR_BEAT_PIN_Pos * 2));

  return (0);
}
