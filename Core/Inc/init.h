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


#define LED_RED_Pin               GPIO_PIN_14
#define LED_RED_Pin_Pos           GPIO_PIN_14_Pos
#define LED_RED_Pin_Mask          GPIO_PIN_14_Mask
#define LED_RED_Port              GPIOB


/* Ethernet GPIO -------------------------------------------------------------*/
#define ETH_REF_CLOCK_Pin         GPIO_PIN_1
#define ETH_REF_CLOCK_Pin_Pos     GPIO_PIN_1_Pos
#define ETH_REF_CLOCK_Pin_Mask    GPIO_PIN_1_Mask
#define ETH_REF_CLOCK_Port        GPIOA

#define ETH_MDIO_Pin              GPIO_PIN_2
#define ETH_MDIO_Pin_Pos          GPIO_PIN_2_Pos
#define ETH_MDIO_Pin_Mask         GPIO_PIN_2_Mask
#define ETH_MDIO_Port             GPIOA

#define ETH_CRS_DV_Pin            GPIO_PIN_7
#define ETH_CRS_DV_Pin_Pos        GPIO_PIN_7_Pos
#define ETH_CRS_DV_Pin_Mask       GPIO_PIN_7_Mask
#define ETH_CRS_DV_Port           GPIOA

#define ETH_TXD1_Pin              GPIO_PIN_13
#define ETH_TXD1_Pin_Pos          GPIO_PIN_13_Pos
#define ETH_TXD1_Pin_Mask         GPIO_PIN_13_Mask
#define ETH_TXD1_Port             GPIOB

#define ETH_MDC_Pin               GPIO_PIN_1
#define ETH_MDC_Pin_Pos           GPIO_PIN_1_Pos
#define ETH_MDC_Pin_Mask          GPIO_PIN_1_Mask
#define ETH_MDC_Port              GPIOC

#define ETH_RXD0_Pin              GPIO_PIN_4
#define ETH_RXD0_Pin_Pos          GPIO_PIN_4_Pos
#define ETH_RXD0_Pin_Mask         GPIO_PIN_4_Mask
#define ETH_RXD0_Port             GPIOC

#define ETH_RXD1_Pin              GPIO_PIN_5
#define ETH_RXD1_Pin_Pos          GPIO_PIN_5_Pos
#define ETH_RXD1_Pin_Mask         GPIO_PIN_5_Mask
#define ETH_RXD1_Port             GPIOC

#define ETH_TX_EN_Pin             GPIO_PIN_11
#define ETH_TX_EN_Pin_Pos         GPIO_PIN_11_Pos
#define ETH_TX_EN_Pin_Mask        GPIO_PIN_11_Mask
#define ETH_TX_EN_Port            GPIOG

#define ETH_TXD0_Pin             GPIO_PIN_13
#define ETH_TXD0_Pin_Pos         GPIO_PIN_13_Pos
#define ETH_TXD0_Pin_Mask        GPIO_PIN_13_Mask
#define ETH_TXD0_Port            GPIOG





int Init_LED();
int Init_ETH();



#ifdef __cplusplus
}
#endif
#endif /* __GENERAL_INIT_H */