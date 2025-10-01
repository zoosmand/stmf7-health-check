/**
  ******************************************************************************
  * @file           : common.h
  * @brief          : Header for common.c file.
  *                   The header file contains the common defines of the 
  *                   application, providing a centralized location for defining
  *                   constants, macros, and other common elements that are used
  *                   throughout the application.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
struct __FILE {
  int handle;
  /* Whatever you require here. If the only file you are using is */
  /* standard output using printf() for debugging, no file handling */
  /* is required. */
};

/* Exported constants --------------------------------------------------------*/

/* Exported macro ------------------------------------------------------------*/
#define FLAG_SET(registry, flag)        SET_BIT(registry, (1 << flag))
#define FLAG_CLR(registry, flag)        CLEAR_BIT(registry, (1 << flag))
#define FLAG_CHECK(registry, flag)      (READ_BIT(registry, (1 << flag)))

#define PIN_H(port, pin)                SET_BIT(port->BSRR, pin)
#define PIN_L(port, pin)                SET_BIT(port->BSRR, (pin << 16))
#define PIN_LEVEL(port, pin)            (READ_BIT(port->IDR, pin))

#define PREG_SET(registry, key)         SET_BIT(registry, (1 << key))
#define PREG_CLR(registry, key)         CLEAR_BIT(registry, (1 << key))
#define PREG_CHECK(registry, key)       (READ_BIT(registry, (1 << key)))



/* Exported structures -------------------------------------------------------*/

/**
  * @brief  RCC Clocks Frequency Structure
  */
typedef struct {
  uint32_t SystemCore;          /*!< System Core frequency */ 
  uint32_t HCLK_Freq;           /*!< HCLK clock frequency */
  uint32_t PCLK1_Freq;          /*!< PCLK1 clock frequency */
  uint32_t PCLK1_Freq_Tim;      /*!< PCLK1 clock frequency for timers */
  uint32_t PCLK2_Freq;          /*!< PCLK2 clock frequency */
  uint32_t PCLK2_Freq_Tim;      /*!< PCLK2 clock frequency for timers */
  uint32_t PLLQ_Freq;           /*!< PLLQ clock frequency for USB bus */
} RCC_ClocksTypeDef;


/* Exported global variables -------------------------------------------------*/
extern RCC_ClocksTypeDef SystemClocks;
extern __IO uint32_t SysTickCnt;


/* Private defines -----------------------------------------------------------*/
/*** NVIC ***/
#define NVIC_PRIORITYGROUP_0         0x00000007U /*!< 0 bits for pre-emption priority 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         0x00000006U /*!< 1 bits for pre-emption priority 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         0x00000005U /*!< 2 bits for pre-emption priority 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         0x00000004U /*!< 3 bits for pre-emption priority 1 bits for subpriority */
#define NVIC_PRIORITYGROUP_4         0x00000003U /*!< 4 bits for pre-emption priority 0 bits for subpriority */

/*** GPIO ***/
#define GPIO_PIN_0                  GPIO_BSRR_BS_0 /*!< Select pin 0 */
#define GPIO_PIN_0_Pos              0
#define GPIO_PIN_0_Mask             0x00000003
#define GPIO_PIN_1                  GPIO_BSRR_BS_1 /*!< Select pin 1 */
#define GPIO_PIN_1_Pos              1
#define GPIO_PIN_1_Mask             0x0000000c
#define GPIO_PIN_2                  GPIO_BSRR_BS_2 /*!< Select pin 2 */
#define GPIO_PIN_2_Pos              2
#define GPIO_PIN_2_Mask             0x00000030
#define GPIO_PIN_3                  GPIO_BSRR_BS_3 /*!< Select pin 3 */
#define GPIO_PIN_3_Pos              3
#define GPIO_PIN_3_Mask             0x000000c0
#define GPIO_PIN_4                  GPIO_BSRR_BS_4 /*!< Select pin 4 */
#define GPIO_PIN_4_Pos              4
#define GPIO_PIN_4_Mask             0x00000300
#define GPIO_PIN_5                  GPIO_BSRR_BS_5 /*!< Select pin 5 */
#define GPIO_PIN_5_Pos              5
#define GPIO_PIN_5_Mask             0x00000c00
#define GPIO_PIN_6                  GPIO_BSRR_BS_6 /*!< Select pin 6 */
#define GPIO_PIN_6_Pos              6
#define GPIO_PIN_6_Mask             0x00003000
#define GPIO_PIN_7                  GPIO_BSRR_BS_7 /*!< Select pin 7 */
#define GPIO_PIN_7_Pos              7
#define GPIO_PIN_7_Mask             0x0000c000
#define GPIO_PIN_8                  GPIO_BSRR_BS_8 /*!< Select pin 8 */
#define GPIO_PIN_8_Pos              8
#define GPIO_PIN_8_Mask             0x00030000
#define GPIO_PIN_9                  GPIO_BSRR_BS_9 /*!< Select pin 9 */
#define GPIO_PIN_9_Pos              9
#define GPIO_PIN_9_Mask             0x000c0000
#define GPIO_PIN_10                 GPIO_BSRR_BS_10 /*!< Select pin 10 */
#define GPIO_PIN_10_Pos             10
#define GPIO_PIN_10_Mask            0x00300000
#define GPIO_PIN_11                 GPIO_BSRR_BS_11 /*!< Select pin 11 */
#define GPIO_PIN_11_Pos             11
#define GPIO_PIN_11_Mask            0x00c00000
#define GPIO_PIN_12                 GPIO_BSRR_BS_12 /*!< Select pin 12 */
#define GPIO_PIN_12_Pos             12
#define GPIO_PIN_12_Mask            0x03000000
#define GPIO_PIN_13                 GPIO_BSRR_BS_13 /*!< Select pin 13 */
#define GPIO_PIN_13_Pos             13
#define GPIO_PIN_13_Mask            0x0c000000
#define GPIO_PIN_14                 GPIO_BSRR_BS_14 /*!< Select pin 14 */
#define GPIO_PIN_14_Pos             14
#define GPIO_PIN_14_Mask            0x30000000
#define GPIO_PIN_15                 GPIO_BSRR_BS_15 /*!< Select pin 15 */
#define GPIO_PIN_15_Pos             15
#define GPIO_PIN_15_Mask            0xc0000000
#define GPIO_PIN_ALL                (uint16_t)0xffff /*!< Select all pins */
/*** GPIO Modes bit definitions ***/
#define _MODE_OUT                   0b01 /*!< GPIO output mode */
#define _MODE_IN                    0b00 /*!< GPIO input mode */
#define _MODE_AF                    0b10 /*!< GPIO alternate function mode */ 
#define _MODE_AN                    0b11 /*!< GPIO analog mode */
#define _OTYPE_PP                   0b0  /*!< GPIO output push-pull mode */ 
#define _OTYPE_OD                   0b1  /*!< GPIO output open-drain mode */
#define _SPEED_L                    0b00 /*!< GPIO low speed ~2MHz */
#define _SPEED_M                    0b01 /*!< GPIO medium speed ~10MHz */
#define _SPEED_H                    0b10 /*!< GPIO high speed ~25MHz */
#define _SPEED_V                    0b11 /*!< GPIO very high speed ~100MHz */
#define _PUPD_NO                    0b00 /*!< GPIO neither pull-up, no pull-down */
#define _PUPD_PU                    0b01 /*!< GPIO pull-up */
#define _PUPD_PD                    0b10 /*!< GPIO pull-down */
/*** GPIO Alternative function defines ***/
#define GPIO_AF_0                   0b0000 /*!< Select alternate function 0 */
#define GPIO_AF_1                   0b0001 /*!< Select alternate function 1 */
#define GPIO_AF_2                   0b0010 /*!< Select alternate function 2 */
#define GPIO_AF_3                   0b0011 /*!< Select alternate function 3 */
#define GPIO_AF_4                   0b0100 /*!< Select alternate function 4 */
#define GPIO_AF_5                   0b0101 /*!< Select alternate function 5 */
#define GPIO_AF_6                   0b0110 /*!< Select alternate function 6 */
#define GPIO_AF_7                   0b0111 /*!< Select alternate function 7 */
#define GPIO_AF_8                   0b1000 /*!< Select alternate function 8 */
#define GPIO_AF_9                   0b1001 /*!< Select alternate function 9 */
#define GPIO_AF_10                  0b1010 /*!< Select alternate function 10 */
#define GPIO_AF_11                  0b1011 /*!< Select alternate function 11 */
#define GPIO_AF_12                  0b1100 /*!< Select alternate function 12 */
#define GPIO_AF_13                  0b1101 /*!< Select alternate function 13 */
#define GPIO_AF_14                  0b1110 /*!< Select alternate function 14 */
#define GPIO_AF_15                  0b1111 /*!< Select alternate function 15 */

/*** EXTI ***/
#define EXTI_TRIGGER_NONE           0b00 /*!< No Trigger Mode */
#define EXTI_TRIGGER_RISING         0b01 /*!< Trigger Rising Mode */
#define EXTI_TRIGGER_FALLING        0b10 /*!< Trigger Falling Mode */
#define EXTI_TRIGGER_BOTH           0b11 /*!< Trigger Rising & Falling Mode */

/*** Independent Watchdog ***/
#define IWDG_KEY_RELOAD             0x0000aaaa               /*!< IWDG Reload Counter Enable   */
#define IWDG_KEY_ENABLE             0x0000cccc               /*!< IWDG Peripheral Enable       */
#define IWDG_KEY_WR_ACCESS_ENABLE   0x00005555               /*!< IWDG KR Write Access Enable  */


/* Peripherals is run flag definitons ----------------------------------------*/
#define LED_BLUE_READY_flag         0
#define ETH_READY_flag              1


/* Exported structures -------------------------------------------------------*/

/* Exported functions prototypes ---------------------------------------------*/
void __attribute__((weak)) Error_Handler(void);
int __attribute__((weak)) putc_dspl(char);
void __attribute__((weak)) _delay_us(uint32_t);
void __attribute__((weak)) _delay_ms(uint32_t);

void LED_Blink(GPIO_TypeDef*, uint16_t);


#ifdef __cplusplus
}
#endif
#endif /* __COMMON_H */