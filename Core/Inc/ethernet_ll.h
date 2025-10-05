/**
  ******************************************************************************
  * @file           : ethernet.h
  * @brief          : Header for ethernet.c file.
  *                   The header file contains the common defines of the 
  *                   ethernet code.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __ETHERNET_H
#define __ETHERNET_H

#ifdef __cplusplus
  extern "C" {
#endif 


/* Includes ------------------------------------------------------------------*/

/* Private includes ----------------------------------------------------------*/
#include "main.h"




// /* --- MACMIIAR bit helpers (F7/F4 style) --- */
// // #define MIIAR_MB         (1u << 0)   /* Busy */
// // #define MIIAR_MW         (1u << 1)   /* 1=Write, 0=Read */
// #define MIIAR_CR_SHIFT   2           /* Clock range */
// #define MIIAR_MR_SHIFT   6           /* PHY reg */
// #define MIIAR_PA_SHIFT   11          /* PHY addr */

// /* HCLK 150..216 MHz -> CR = 0b100 => divide ≈102 (MDC ≤ 2.5 MHz) */
// #define MIIAR_CR_102     (0x4u << MIIAR_CR_SHIFT)




// #define MIIAR_MB_Pos  0
// #define MIIAR_MB      (1 << MIIAR_MB_Pos)

// #define MIIAR_MW_Pos  1
// #define MIIAR_MW      (1 << MIIAR_MW_Pos)

// #define MIIAR_CR(x)   ((x) & (7u << 2))
// #define MIIAR_MR(r)   ((uint32_t)(r) << 6)
// #define MIIAR_PA(p)   ((uint32_t)(p) << 11)


#define DCACHE_LINE 32u

#define PBUF_LINK_ENCAPSULATION_HLEN    0
#define ETH_PAD_SIZE                    0
#define PBUF_LINK_HLEN                  (14 + ETH_PAD_SIZE)
#define LWIP_PBUF_REF_T                 u8_t
#define LWIP_PBUF_CUSTOM_DATA



typedef struct {
  volatile uint32_t RDES0;
  volatile uint32_t RDES1;
  volatile uint32_t RDES2;
  volatile uint32_t RDES3;
} eth_rx_desc_t;

typedef struct {
  volatile uint32_t TDES0;
  volatile uint32_t TDES1;
  volatile uint32_t TDES2;
  volatile uint32_t TDES3;
} eth_tx_desc_t;



/* --- Bits (common ST MAC) --- */
#define RDES0_OWN        (1u << 31)
#define RDES1_RCH        (1u << 14)      /* chained */
#define RDES1_RBS1_MASK  (0x1FFFu)       /* RX buf1 size (max 8191) */

#define TDES0_OWN        (1u << 31)
#define TDES0_LS         (1u << 29)      /* last segment */
#define TDES0_FS         (1u << 28)      /* first segment */
#define TDES0_CIC_FULL   (3u << 22)      /* checksum insertion (IP+TCP/UDP) */
#define TDES1_TCH        (1u << 20)      /* chained */
#define TDES1_TBS1_MASK  (0x1FFFu)       /* TX buf1 size */


/* Ring sizes and buffers */
#define ETH_RX_DESC_CNT  8
#define ETH_TX_DESC_CNT  8
#define ETH_RX_BUF_SIZE  1536            /* ≥ largest frame + margin */
#define ETH_TX_BUF_SIZE  1536




/* Example phy utils */
#define PHY_BMCR        0x00
#define PHY_BMSR        0x01
#define PHY_SR          0x1F   /* LAN8742: Special Control/Status */

typedef enum { PHY_SPD_10, PHY_SPD_100 } phy_speed_t;
typedef enum { PHY_DUP_HALF, PHY_DUP_FULL } phy_duplex_t;






/* Exported functions prototypes ---------------------------------------------*/
int Init_ETH_RMII(uint8_t, const uint8_t[6]);


#ifdef __cplusplus
}
#endif
#endif /* __ETHERNET_H */
