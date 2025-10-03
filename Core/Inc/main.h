/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the defines of the application.
  ******************************************************************************
  * @attention
  *
  * 
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H


#ifdef __cplusplus
extern "C" {
#endif


/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
//
#include "stm32f7xx.h"
#include "stm32f7xx_it.h"

/* Private includes ----------------------------------------------------------*/
#include "common.h"
#include "init_ll.h"
#include "ethernet_ll.h"
#include "lan8742.h"
#include "lwipopts.h"
#include "cmsis_os.h"

#include "lwip/arch.h"
#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"

#include "lwip/err.h"
#include "lwip/netif.h"



/* Exported types ------------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/

/* Extern global variables ---------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/

/* Private defines -----------------------------------------------------------*/

/* Public defines ------------------------------------------------------------*/


#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
