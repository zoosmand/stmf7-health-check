
/**
  ******************************************************************************
  * @file           : init.c
  * @brief          : The C code file provides a collection of peripherals
  *                   initialization procedures.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
 
/* Includes ------------------------------------------------------------------*/
#include "init.h"

/* Global variables ----------------------------------------------------------*/




int Init_LED(void) {
  /*** GPIO LED -  PB0, PB7, PB14 ***/
  /* Mode */
  MODIFY_REG(LED_BLUE_Port->MODER, LED_BLUE_Pin_Mask, (_MODE_OUT << LED_BLUE_Pin_Pos * 2));
  /* Speed */
  // MODIFY_REG(LED_BLUE_Port->OSPEEDR, LED_BLUE_Pin_Mask, (_SPEED_L << LED_BLUE_Pin_Pos * 2));
  // /* Output type */
  // MODIFY_REG(LED_BLUE_Port->OTYPER, (_OTYPE_PP << LED_BLUE_Pin_Mask), (_OTYPE_PP << LED_BLUE_Pin_Pos));
  // /* Push mode */
  // MODIFY_REG(LED_BLUE_Port->PUPDR, LED_BLUE_Pin_Mask, (_PUPD_NO << LED_BLUE_Pin_Pos * 2));

  return (0);
}