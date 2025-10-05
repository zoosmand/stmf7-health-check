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


int LED_Init(GPIO_TypeDef *port, uint16_t pin) {
    uint32_t shift;
    #define GPIO_MODE_OUTPUT_PP_2MHZ (_IOS_2 | _GPO_PP)

    if (pin < 8) {
        shift = pin * 4;
        MODIFY_REG(port->CRL, (0xf << shift), (GPIO_MODE_OUTPUT_PP_2MHZ << shift));
    } else {
        shift = (pin - 8) * 4;
        MODIFY_REG(port->CRH, (0xf << shift), (GPIO_MODE_OUTPUT_PP_2MHZ << shift));
    }

    return 0;
}


int OneWire_Init(GPIO_TypeDef *port, uint16_t pin) {
    uint32_t shift;
    #define GPIO_MODE_OUTPUT_OD_10MHZ (_IOS_10 | _GPO_OD)

    if (pin < 8) {
        shift = pin * 4;
        MODIFY_REG(port->CRL, (0xf << shift), (GPIO_MODE_OUTPUT_OD_10MHZ << shift));
    } else {
        shift = (pin - 8) * 4;
        MODIFY_REG(port->CRH, (0xf << shift), (GPIO_MODE_OUTPUT_OD_10MHZ << shift));
    }
    PIN_H(port, pin);

    return 0;
}

