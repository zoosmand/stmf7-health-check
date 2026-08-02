/**
  ******************************************************************************
  * @file           : init_ll.c
  * @brief          : Board-level peripheral initialization.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 01.10.2025
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2026 Dmitry Slobodchikov
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "init_ll.h"

#include "common.h"

#include <stdio.h>

#define ETHERNET_PHY_ADDRESS 0U
#define ETHERNET_GPIO_AF     11U
#define DIAGNOSTIC_GPIO_AF   7U
#define DIAGNOSTIC_TX_PIN    8U
#define DIAGNOSTIC_RX_PIN    9U

/**
  * @brief One RMII signal's GPIO assignment.
  * @param port (GPIO_TypeDef*) GPIO peripheral containing the signal.
  * @param pinPosition (uint8_t) GPIO pin position from 0 through 15.
  */
typedef struct {
  GPIO_TypeDef* port;
  uint8_t pinPosition;
} BoardEthernetPin_TypeDef;

static const uint8_t ethernetMacAddress[6] = {
  0x00U, 0x80U, 0xAAU, 0xBBU, 0xCCU, 0xDDU
};

static const BoardEthernetPin_TypeDef ethernetPins[] = {
  {GPIOA, 1U},
  {GPIOA, 2U},
  {GPIOA, 7U},
  {GPIOB, 13U},
  {GPIOC, 1U},
  {GPIOC, 4U},
  {GPIOC, 5U},
  {GPIOG, 11U},
  {GPIOG, 13U},
};

static void board_ConfigureAlternatePin(
  GPIO_TypeDef* port,
  uint32_t pinPosition,
  uint32_t alternateFunction
);

void Board_InitDiagnosticUart(void) {
  SET_BIT(RCC->AHB1ENR, RCC_AHB1ENR_GPIODEN);
  SET_BIT(RCC->APB1ENR, RCC_APB1ENR_USART3EN);
  (void)RCC->APB1ENR;

  board_ConfigureAlternatePin(GPIOD, DIAGNOSTIC_TX_PIN, DIAGNOSTIC_GPIO_AF);
  board_ConfigureAlternatePin(GPIOD, DIAGNOSTIC_RX_PIN, DIAGNOSTIC_GPIO_AF);

  CLEAR_BIT(USART3->CR1, USART_CR1_UE);
  USART3->CR1 = USART_CR1_TE | USART_CR1_RE;
  USART3->CR2 = 0U;
  USART3->CR3 = 0U;
  USART3->BRR = (systemClocks.pclk1Hz + (BOARD_DIAGNOSTIC_BAUD_RATE / 2U))
    / BOARD_DIAGNOSTIC_BAUD_RATE;
  SET_BIT(USART3->CR1, USART_CR1_UE);
}

void Board_PrintConfiguration(void) {
  printf("\nSTM32F767 health checker startup\n");
  printf("Board: ST NUCLEO-F767ZI (MB1137)\n");
  printf("MCU: STM32F767ZIT6, Cortex-M7\n");
  printf(
    "Clocks: core=%lu MHz, APB1=%lu MHz, APB2=%lu MHz\n",
    (unsigned long)(systemClocks.systemCoreHz / 1000000U),
    (unsigned long)(systemClocks.pclk1Hz / 1000000U),
    (unsigned long)(systemClocks.pclk2Hz / 1000000U)
  );
  printf(
    "Console: ST-LINK VCP, USART3 PD8/PD9, %lu 8N1\n",
    (unsigned long)BOARD_DIAGNOSTIC_BAUD_RATE
  );
  printf("Ethernet: LAN8742A, RMII, PHY address %u\n", ETHERNET_PHY_ADDRESS);
  printf("External flash: W25Q64, SPI1 PB3/PB4/PB5, NSS PA4\n");
}

Ethernet_StatusTypeDef Board_InitEthernet(void) {
  for (uint32_t index = 0U;
       index < (sizeof(ethernetPins) / sizeof(ethernetPins[0]));
       index++) {
    board_ConfigureAlternatePin(
      ethernetPins[index].port,
      ethernetPins[index].pinPosition,
      ETHERNET_GPIO_AF
    );
  }

  return EthernetMac_Init(ETHERNET_PHY_ADDRESS, ethernetMacAddress);
}

/**
  * @brief Configure a GPIO for a very-high-speed push-pull alternate function.
  * @param port (GPIO_TypeDef*) Non-null GPIO peripheral.
  * @param pinPosition (uint32_t) GPIO pin position from 0 through 15.
  * @param alternateFunction (uint32_t) Alternate-function number from 0 to 15.
  */
static void board_ConfigureAlternatePin(
  GPIO_TypeDef* port,
  uint32_t pinPosition,
  uint32_t alternateFunction
) {
  uint32_t modeShift = pinPosition * 2U;
  uint32_t modeMask = 0x3UL << modeShift;
  uint32_t pinMask = 1UL << pinPosition;
  uint32_t alternateIndex = pinPosition / 8U;
  uint32_t alternateShift = (pinPosition % 8U) * 4U;

  MODIFY_REG(
    port->MODER,
    modeMask,
    GPIO_MODE_ALTERNATE << modeShift
  );
  MODIFY_REG(
    port->OSPEEDR,
    modeMask,
    GPIO_SPEED_VERY_HIGH << modeShift
  );
  CLEAR_BIT(port->OTYPER, pinMask);
  CLEAR_BIT(port->PUPDR, modeMask);
  MODIFY_REG(
    port->AFR[alternateIndex],
    0xFUL << alternateShift,
    alternateFunction << alternateShift
  );
}
