
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
#include "init_ll.h"

/* Global variables ----------------------------------------------------------*/
const uint8_t eth_mac_addr[6] = {0x00, 0x80, 0xaa, 0xbb, 0xcc, 0xdd};



int Init_LED(void) {
  /*** GPIO LED -  PB7 ***/
  /* Mode */
  MODIFY_REG(LED_RED_Port->MODER, LED_RED_Pin_Mask, (_MODE_OUT << LED_RED_Pin_Pos * 2));
  /* Speed */
  // MODIFY_REG(LED_BLUE_Port->OSPEEDR, LED_BLUE_Pin_Mask, (_SPEED_L << LED_BLUE_Pin_Pos * 2));
  // /* Output type */
  // MODIFY_REG(LED_BLUE_Port->OTYPER, (_OTYPE_PP << LED_BLUE_Pin_Mask), (_OTYPE_PP << LED_BLUE_Pin_Pos));
  // /* Push mode */
  // MODIFY_REG(LED_BLUE_Port->PUPDR, LED_BLUE_Pin_Mask, (_PUPD_NO << LED_BLUE_Pin_Pos * 2));

  return (0);
}


int Init_ETH_LL(void) {

  /*** ETH_REF_CLOCK / PA1 ***/
  MODIFY_REG(ETH_REF_CLOCK_Port->MODER, ETH_REF_CLOCK_Pin_Mask, (_MODE_AF << (ETH_REF_CLOCK_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_REF_CLOCK_Port->OSPEEDR, ETH_REF_CLOCK_Pin_Mask, (_SPEED_V << (ETH_REF_CLOCK_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_REF_CLOCK_Port->OTYPER, ETH_REF_CLOCK_Pin, (_OTYPE_PP << ETH_REF_CLOCK_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_REF_CLOCK_Port->PUPDR, ETH_REF_CLOCK_Pin_Mask, (_PUPD_NO << (ETH_REF_CLOCK_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_REF_CLOCK_Port->AFR[0], (0xf << (ETH_REF_CLOCK_Pin_Pos * 4)), (GPIO_AF_11 << (ETH_REF_CLOCK_Pin_Pos * 4)));


  /*** ETH_MDIO / PA2 ***/
  MODIFY_REG(ETH_MDIO_Port->MODER, ETH_MDIO_Pin_Mask, (_MODE_AF << (ETH_MDIO_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_MDIO_Port->OSPEEDR, ETH_MDIO_Pin_Mask, (_SPEED_V << (ETH_MDIO_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_MDIO_Port->OTYPER, ETH_MDIO_Pin, (_OTYPE_PP << ETH_MDIO_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_MDIO_Port->PUPDR, ETH_MDIO_Pin_Mask, (_PUPD_NO << (ETH_MDIO_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_MDIO_Port->AFR[0], (0xf << (ETH_MDIO_Pin_Pos * 4)), (GPIO_AF_11 << (ETH_MDIO_Pin_Pos * 4)));


  /*** ETH_CRS_DV / PA7 ***/
  MODIFY_REG(ETH_CRS_DV_Port->MODER, ETH_CRS_DV_Pin_Mask, (_MODE_AF << (ETH_CRS_DV_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_CRS_DV_Port->OSPEEDR, ETH_CRS_DV_Pin_Mask, (_SPEED_V << (ETH_CRS_DV_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_CRS_DV_Port->OTYPER, ETH_CRS_DV_Pin, (_OTYPE_PP << ETH_CRS_DV_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_CRS_DV_Port->PUPDR, ETH_CRS_DV_Pin_Mask, (_PUPD_NO << (ETH_CRS_DV_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_CRS_DV_Port->AFR[0], (0xf << (ETH_CRS_DV_Pin_Pos * 4)), (GPIO_AF_11 << (ETH_CRS_DV_Pin_Pos * 4)));


  /*** ETH_TXD1 / PB13 ***/
  MODIFY_REG(ETH_TXD1_Port->MODER, ETH_TXD1_Pin_Mask, (_MODE_AF << (ETH_TXD1_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_TXD1_Port->OSPEEDR, ETH_TXD1_Pin_Mask, (_SPEED_V << (ETH_TXD1_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_TXD1_Port->OTYPER, ETH_TXD1_Pin, (_OTYPE_PP << ETH_TXD1_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_TXD1_Port->PUPDR, ETH_TXD1_Pin_Mask, (_PUPD_NO << (ETH_TXD1_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_TXD1_Port->AFR[1], (0xf << ((ETH_TXD1_Pin_Pos - 8) * 4)), (GPIO_AF_11 << ((ETH_TXD1_Pin_Pos - 8) * 4)));


  /*** ETH_MDC / PC1 ***/
  MODIFY_REG(ETH_MDC_Port->MODER, ETH_MDC_Pin_Mask, (_MODE_AF << (ETH_MDC_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_MDC_Port->OSPEEDR, ETH_MDC_Pin_Mask, (_SPEED_V << (ETH_MDC_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_MDC_Port->OTYPER, ETH_MDC_Pin, (_OTYPE_PP << ETH_MDC_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_MDC_Port->PUPDR, ETH_MDC_Pin_Mask, (_PUPD_NO << (ETH_MDC_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_MDC_Port->AFR[0], (0xf << (ETH_MDC_Pin_Pos * 4)), (GPIO_AF_11 << (ETH_MDC_Pin_Pos * 4)));


  /*** ETH_RXD0 / PC4 ***/
  MODIFY_REG(ETH_RXD0_Port->MODER, ETH_RXD0_Pin_Mask, (_MODE_AF << (ETH_RXD0_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_RXD0_Port->OSPEEDR, ETH_RXD0_Pin_Mask, (_SPEED_V << (ETH_RXD0_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_RXD0_Port->OTYPER, ETH_RXD0_Pin, (_OTYPE_PP << ETH_RXD0_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_RXD0_Port->PUPDR, ETH_RXD0_Pin_Mask, (_PUPD_NO << (ETH_RXD0_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_RXD0_Port->AFR[0], (0xf << (ETH_RXD0_Pin_Pos * 4)), (GPIO_AF_11 << (ETH_RXD0_Pin_Pos * 4)));


  /*** ETH_RXD1 / PC5 ***/
  MODIFY_REG(ETH_RXD1_Port->MODER, ETH_RXD1_Pin_Mask, (_MODE_AF << (ETH_RXD1_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_RXD1_Port->OSPEEDR, ETH_RXD1_Pin_Mask, (_SPEED_V << (ETH_RXD1_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_RXD1_Port->OTYPER, ETH_RXD1_Pin, (_OTYPE_PP << ETH_RXD1_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_RXD1_Port->PUPDR, ETH_RXD1_Pin_Mask, (_PUPD_NO << (ETH_RXD1_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_RXD1_Port->AFR[0], (0xf << (ETH_RXD1_Pin_Pos * 4)), (GPIO_AF_11 << (ETH_RXD1_Pin_Pos * 4)));


  /*** ETH_TX_EN / PG11 ***/
  MODIFY_REG(ETH_TX_EN_Port->MODER, ETH_TX_EN_Pin_Mask, (_MODE_AF << (ETH_TX_EN_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_TX_EN_Port->OSPEEDR, ETH_TX_EN_Pin_Mask, (_SPEED_V << (ETH_TX_EN_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_TX_EN_Port->OTYPER, ETH_TX_EN_Pin, (_OTYPE_PP << ETH_TX_EN_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_TX_EN_Port->PUPDR, ETH_TX_EN_Pin_Mask, (_PUPD_NO << (ETH_TX_EN_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_TX_EN_Port->AFR[1], (0xf << ((ETH_TX_EN_Pin_Pos -8) * 4)), (GPIO_AF_11 << ((ETH_TX_EN_Pin_Pos -8) * 4)));


  /*** ETH_TXD0 / PG13 ***/
  MODIFY_REG(ETH_TXD0_Port->MODER, ETH_TXD0_Pin_Mask, (_MODE_AF << (ETH_TXD0_Pin_Pos * 2)));
  /* Speed */
  MODIFY_REG(ETH_TXD0_Port->OSPEEDR, ETH_TXD0_Pin_Mask, (_SPEED_V << (ETH_TXD0_Pin_Pos * 2)));
  /* Output type */
  MODIFY_REG(ETH_TXD0_Port->OTYPER, ETH_TXD0_Pin, (_OTYPE_PP << ETH_TXD0_Pin_Pos));
  /* Push mode */
  MODIFY_REG(ETH_TXD0_Port->PUPDR, ETH_TXD0_Pin_Mask, (_PUPD_NO << (ETH_TXD0_Pin_Pos * 2)));
  /* Alternate function */
  MODIFY_REG(ETH_TXD0_Port->AFR[1], (0xf << ((ETH_TXD0_Pin_Pos -8) * 4)), (GPIO_AF_11 << ((ETH_TXD0_Pin_Pos -8) * 4)));



  
  
  for (uint8_t i = 0; i < sizeof(eth_mac_addr); i++) {
    gnetif.hwaddr[i] = eth_mac_addr[i];
  }


  if (Init_ETH_RMII(LAN8742_MAX_DEV_ADDR, eth_mac_addr)) return (1);

  return (0);
}