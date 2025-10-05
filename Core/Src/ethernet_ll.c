/**
  ******************************************************************************
  * @file           : ethernet.c
  * @brief          : The C code file provides a collection of ethernet code.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
 
/* Includes ------------------------------------------------------------------*/
#include "ethernet_ll.h"

/* Global variables ----------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
static uint32_t eth_mdio_cr_from_hclk(uint32_t);


__STATIC_INLINE void mdio_write(uint8_t, uint8_t, uint16_t);
__STATIC_INLINE uint16_t mdio_read(uint8_t, uint8_t);

static void phy_read_link(uint8_t, int*, phy_speed_t*, phy_duplex_t*);
static void mac_apply_speed_duplex(phy_speed_t, phy_duplex_t);



/* Place in **AXI SRAM**, 32-byte aligned; mark non-cacheable or do cache maintenance */
__attribute__((section(".eth"), aligned(32))) static eth_rx_desc_t rx_desc[ETH_RX_DESC_CNT];
__attribute__((section(".eth"), aligned(32))) static eth_tx_desc_t tx_desc[ETH_TX_DESC_CNT];

__attribute__((section(".eth"), aligned(32))) static uint8_t rx_buf[ETH_RX_DESC_CNT][ETH_RX_BUF_SIZE];
__attribute__((section(".eth"), aligned(32))) static uint8_t tx_buf[ETH_TX_DESC_CNT][ETH_TX_BUF_SIZE];

static uint32_t rx_idx = 0, tx_idx = 0;






__STATIC_INLINE void mdio_write(uint8_t phy, uint8_t reg, uint16_t val) {
  while (!(PREG_CHECK(ETH->MACMIIAR, ETH_MACMIIAR_MB_Pos)));

  ETH->MACMIIDR = val;

  SET_BIT(ETH->MACMIIAR, ( 
      ETH_MACMIIAR_MB
    | ETH_MACMIIAR_MW
    | (0b100 << ETH_MACMIIAR_CR_Pos)
    | (reg << ETH_MACMIIAR_MR_Pos)
    | (phy << ETH_MACMIIAR_PA_Pos)
  ));

  while (!(PREG_CHECK(ETH->MACMIIAR, ETH_MACMIIAR_MB_Pos)));
}



__STATIC_INLINE uint16_t mdio_read(uint8_t phy, uint8_t reg) {
  while (!(PREG_CHECK(ETH->MACMIIAR, ETH_MACMIIAR_MB_Pos)));
  
  SET_BIT(ETH->MACMIIAR, (
    ETH_MACMIIAR_MB
    | (0b100 << ETH_MACMIIAR_CR_Pos)
    | (reg << ETH_MACMIIAR_MR_Pos)
    | (phy << ETH_MACMIIAR_PA_Pos)
  ));
  
  while (!(PREG_CHECK(ETH->MACMIIAR, ETH_MACMIIAR_MB_Pos)));
  return (uint16_t)ETH->MACMIIDR;
}





int Init_ETH_RMII(uint8_t phy_addr, const uint8_t mac[6]) {
  /* --- Clocks & RMII select --- */
  SET_BIT(SYSCFG->PMC, SYSCFG_PMC_MII_RMII_SEL);

  /* Optional: MAC reset */
  SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_ETHMACRST);
  CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_ETHMACRST);

  /* --- DMA software reset --- */
  PREG_SET(ETH->DMABMR, ETH_DMABMR_SR_Pos);
  while (!(PREG_CHECK(ETH->DMABMR, ETH_DMABMR_SR_Pos)));


  /* --- Basic MAC config: disable TX/RX during setup --- */
  CLEAR_BIT(ETH->MACCR, (ETH_MACCR_TE | ETH_MACCR_RE));


  /* Frame filter: receive own, RA=0, normal perfect filtering */
  ETH->MACFFR = 0;

  /* Flow control off by default */
  ETH->MACFCR = 0;

  /* Program MAC address 0 (your unicast) */
  ETH->MACA0HR = (uint32_t)((mac[5] << 8) | mac[4]);
  ETH->MACA0LR = (uint32_t)((mac[3] << 24) | (mac[2] << 16) | (mac[1] << 8) | mac[0]);

  /* --- DMA operating mode: store & forward simplifies checksumming --- */
  /* RX/TX store & forward */
  PREG_SET(ETH->DMAOMR, (
      ETH_DMAOMR_RSF
    | ETH_DMAOMR_TSF
  ));

  /* DMA bus mode: fixed burst, 32-beat PBL, AAB, 4xPBL when Rx */
  PREG_SET(ETH->DMABMR, (
      ETH_DMABMR_AAB
    | ETH_DMABMR_FB
    | ETH_DMABMR_RDP_32Beat
    | ETH_DMABMR_PBL_32Beat
  ));

  /* --- Descriptor rings --- */
  for (uint32_t i = 0; i < ETH_RX_DESC_CNT; ++i) {
    rx_desc[i].RDES0 = RDES0_OWN;
    rx_desc[i].RDES1 = (ETH_RX_BUF_SIZE & RDES1_RBS1_MASK) | RDES1_RCH;
    rx_desc[i].RDES2 = (uint32_t)&rx_buf[i][0];
    rx_desc[i].RDES3 = (uint32_t)&rx_desc[(i + 1) % ETH_RX_DESC_CNT];
  }
  for (uint32_t i = 0; i < ETH_TX_DESC_CNT; ++i) {
    tx_desc[i].TDES0 = 0; /* not owned by DMA yet */
    tx_desc[i].TDES1 = TDES1_TCH;
    tx_desc[i].TDES2 = (uint32_t)&tx_buf[i][0];
    tx_desc[i].TDES3 = (uint32_t)&tx_desc[(i + 1) % ETH_TX_DESC_CNT];
  }
  rx_idx = tx_idx = 0;

  /* Base addresses */
  ETH->DMARDLAR = (uint32_t)&rx_desc[0];
  ETH->DMATDLAR = (uint32_t)&tx_desc[0];

  /* (If D-Cache ON) Clean DCache for descriptors & RX buffers here */

  /* --- PHY: start autoneg (generic) --- */
  /* Write BMCR (reg 0): set AN enable + restart AN */
  mdio_write(phy_addr, 0x00, 0x1200);  // optional: depends on PHY

  
  /* Wait link, then read speed/duplex from PHY specific status.
      Example (pseudo): if 100Mbps full-duplex: set MACCR FES|DM */
  uint32_t maccr = ETH->MACCR;
  maccr |= ETH_MACCR_IPCO;   /* (optional) IPv4 checksum offload in MAC */
  maccr |= ETH_MACCR_DM;     /* assume full-duplex initially */
  maccr |= ETH_MACCR_FES;    /* assume 100 Mbps initially */
  /* TODO: read your PHY to set DM/FES correctly */
  ETH->MACCR = maccr;

  /* --- Start DMA reception/transmission --- */
  ETH->DMAOMR |= ETH_DMAOMR_SR;   /* Start Rx */
  ETH->DMAOMR |= ETH_DMAOMR_ST;   /* Start Tx */

  /* Enable MAC Rx/Tx */
  PREG_SET(ETH->MACCR, (
      ETH_MACCR_RE
    | ETH_MACCR_TE
  ));

  return 0;
}





static void phy_read_link(uint8_t phy, int *link_up, phy_speed_t *spd, phy_duplex_t *dup) {
  /* Read twice to clear latched bits (standard practice) */
  (void)mdio_read(phy, PHY_BMSR);
  uint16_t bmsr = mdio_read(phy, PHY_BMSR);
  *link_up = (bmsr & (1u << 2)) ? 1 : 0;  /* Link Status bit */

  /* LAN8742 special status gives resolved speed/duplex when link up.
      (Consult datasheet for exact bit positions on your board rev.) */
  uint16_t sr = mdio_read(phy, PHY_SR);
  /* Example mapping (adjust if your datasheet differs):
      bit[10]=Speed (1=100,0=10), bit[13]=Duplex (1=Full), bit[2]=Link */
  *spd =  (sr & (1u << 10)) ? PHY_SPD_100 : PHY_SPD_10;
  *dup =  (sr & (1u << 13)) ? PHY_DUP_FULL : PHY_DUP_HALF;
  if (!(sr & (1u << 2))) *link_up = 0;
}



/* Apply to MAC */
static void mac_apply_speed_duplex(phy_speed_t spd, phy_duplex_t dup) {
  uint32_t maccr = ETH->MACCR;
  maccr &= ~(ETH_MACCR_DM | ETH_MACCR_FES);
  if (dup == PHY_DUP_FULL) maccr |= ETH_MACCR_DM;
  if (spd == PHY_SPD_100)  maccr |= ETH_MACCR_FES;
  ETH->MACCR = maccr;
}




/* Example helpers (32-byte cache line) */
__STATIC_INLINE uintptr_t align_down(uintptr_t a) {
  return a & ~(uintptr_t)(DCACHE_LINE - 1u);
}


__STATIC_INLINE uintptr_t align_up (uintptr_t a) {
  return (a + DCACHE_LINE - 1u) & ~(uintptr_t)(DCACHE_LINE - 1u);
}


__STATIC_INLINE void dcache_clean(void *addr, size_t len) {
    uintptr_t a = (uintptr_t)addr;
    SCB_CleanDCache_by_Addr((void*)align_down(a), (int)(align_up(a + len) - align_down(a)));
}

__STATIC_INLINE void dcache_invalidate(void *addr, size_t len) {
    uintptr_t a = (uintptr_t)addr;
    SCB_InvalidateDCache_by_Addr((void*)align_down(a), (int)(align_up(a + len) - align_down(a)));
}



