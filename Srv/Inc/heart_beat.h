/**
  ******************************************************************************
  * @file           : heart_beat.h
  * @brief          : Header for heart_beat.c file.
  *                   This file contains the common defines for LED blinking 
  *                   that represent the system health.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
  /* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __HEART_BEAT_H
#define __HEART_BEAT_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "main.h"


#define HEAR_BEAT_PORT   GPIOC
#define HEAR_BEAT_PIN    GPIO_PIN_13


/* Exported functions prototypes ---------------------------------------------*/

/**
  * @brief  Heartbeat LED blinking service
  * @param  none
  * @retval none
 */
void HeartBeatService(void);


#ifdef __cplusplus
}
#endif

#endif /* __HEART_BEAT_H */