/**
  ******************************************************************************
  * @file           : init.h
  * @brief          : Header for init.c file.
  *                   The header file contains the common defines of the 
  *                   peripherals initialization.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __GENERAL_INIT_H
#define __GENERAL_INIT_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include "main.h"


#define LED_BLUE_Pin        GPIO_PIN_7
#define LED_BLUE_Pin_Pos    GPIO_PIN_7_Pos
#define LED_BLUE_Pin_Mask   GPIO_PIN_7_Mask
#define LED_BLUE_Port       GPIOB


int Init_LED();



#ifdef __cplusplus
}
#endif
#endif /* __GENERAL_INIT_H */