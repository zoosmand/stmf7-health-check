/**
  ******************************************************************************
  * @file           : ethernet_ll.c
  * @brief          : STM32F767 Ethernet MAC and LAN8742 low-level setup.
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

#include "ethernet_ll.h"

#include "common.h"
#include "lan8742.h"

#include <stddef.h>
#include <string.h>

#define ETHERNET_RX_DESCRIPTOR_COUNT 8U
#define ETHERNET_TX_DESCRIPTOR_COUNT 8U
#define ETHERNET_BUFFER_SIZE         1536U
#define ETHERNET_REGISTER_TIMEOUT    1000000U

#define ETHERNET_RX_OWN              (1UL << 31U)
#define ETHERNET_RX_CHAINED          (1UL << 14U)
#define ETHERNET_RX_BUFFER_MASK      0x1FFFUL
#define ETHERNET_TX_CHAINED          (1UL << 20U)
#define ETHERNET_TX_FIRST_SEGMENT    (1UL << 28U)
#define ETHERNET_TX_LAST_SEGMENT     (1UL << 29U)
#define ETHERNET_TX_OWN              (1UL << 31U)
#define ETHERNET_TX_CHECKSUM_FULL    (3UL << 22U)
#define ETHERNET_RX_ERROR            (1UL << 15U)
#define ETHERNET_RX_FIRST_SEGMENT    (1UL << 9U)
#define ETHERNET_RX_LAST_SEGMENT     (1UL << 8U)
#define ETHERNET_RX_FRAME_LENGTH(status) (((status) >> 16U) & 0x3FFFU)

/**
  * @brief Four-word STM32F7 Ethernet DMA descriptor.
  * @param status (uint32_t) DMA ownership and frame status bits.
  * @param control (uint32_t) Buffer length and descriptor control bits.
  * @param bufferAddress (uint32_t) DMA buffer address.
  * @param nextDescriptor (uint32_t) Next descriptor address in the ring.
  */
typedef struct {
  volatile uint32_t status;
  volatile uint32_t control;
  volatile uint32_t bufferAddress;
  volatile uint32_t nextDescriptor;
} EthernetDmaDescriptor_TypeDef;

__attribute__((section(".eth"), aligned(32)))
static EthernetDmaDescriptor_TypeDef receiveDescriptors[ETHERNET_RX_DESCRIPTOR_COUNT];
__attribute__((section(".eth"), aligned(32)))
static EthernetDmaDescriptor_TypeDef transmitDescriptors[ETHERNET_TX_DESCRIPTOR_COUNT];
__attribute__((section(".eth"), aligned(32)))
static uint8_t receiveBuffers[ETHERNET_RX_DESCRIPTOR_COUNT][ETHERNET_BUFFER_SIZE];
__attribute__((section(".eth"), aligned(32)))
static uint8_t transmitBuffers[ETHERNET_TX_DESCRIPTOR_COUNT][ETHERNET_BUFFER_SIZE];
static uint8_t ethernetPhyAddress;
static uint32_t receiveDescriptorIndex;
static uint32_t transmitDescriptorIndex;

static Ethernet_StatusTypeDef ethernetMac_WaitForClear(
  volatile uint32_t* registerAddress,
  uint32_t mask,
  Ethernet_StatusTypeDef timeoutStatus
);
static Ethernet_StatusTypeDef ethernetMac_WritePhy(
  uint8_t phyAddress,
  uint8_t registerAddress,
  uint16_t value
);
static Ethernet_StatusTypeDef ethernetMac_ReadPhy(
  uint8_t phyAddress,
  uint8_t registerAddress,
  uint16_t* value
);
static Ethernet_StatusTypeDef ethernetMac_ConfigureLink(uint8_t phyAddress);

Ethernet_StatusTypeDef EthernetMac_Init(
  uint8_t phyAddress,
  const uint8_t macAddress[6]
) {
  if ((macAddress == NULL) || (phyAddress > 31U))
    return ETHERNET_STATUS_INVALID_ARGUMENT;
  ethernetPhyAddress = phyAddress;
  receiveDescriptorIndex = 0U;
  transmitDescriptorIndex = 0U;

  SET_BIT(SYSCFG->PMC, SYSCFG_PMC_MII_RMII_SEL);
  SET_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_ETHMACRST);
  CLEAR_BIT(RCC->AHB1RSTR, RCC_AHB1RSTR_ETHMACRST);

  SET_BIT(ETH->DMABMR, ETH_DMABMR_SR);
  Ethernet_StatusTypeDef status = ethernetMac_WaitForClear(
    &ETH->DMABMR,
    ETH_DMABMR_SR,
    ETHERNET_STATUS_DMA_TIMEOUT
  );
  if (status != ETHERNET_STATUS_OK)
    return status;

  CLEAR_BIT(ETH->MACCR, ETH_MACCR_TE | ETH_MACCR_RE);
  ETH->MACFFR = 0U;
  ETH->MACFCR = 0U;
  ETH->MACA0HR = ((uint32_t)macAddress[5] << 8U) | macAddress[4];
  ETH->MACA0LR = ((uint32_t)macAddress[3] << 24U)
    | ((uint32_t)macAddress[2] << 16U)
    | ((uint32_t)macAddress[1] << 8U)
    | macAddress[0];

  SET_BIT(ETH->DMAOMR, ETH_DMAOMR_RSF | ETH_DMAOMR_TSF);
  MODIFY_REG(
    ETH->DMABMR,
    ETH_DMABMR_AAB | ETH_DMABMR_FB
      | ETH_DMABMR_RDP | ETH_DMABMR_PBL,
    ETH_DMABMR_AAB | ETH_DMABMR_FB
      | ETH_DMABMR_RDP_32Beat | ETH_DMABMR_PBL_32Beat
  );

  for (uint32_t index = 0U; index < ETHERNET_RX_DESCRIPTOR_COUNT; index++) {
    receiveDescriptors[index].status = ETHERNET_RX_OWN;
    receiveDescriptors[index].control =
      (ETHERNET_BUFFER_SIZE & ETHERNET_RX_BUFFER_MASK) | ETHERNET_RX_CHAINED;
    receiveDescriptors[index].bufferAddress =
      (uint32_t)&receiveBuffers[index][0];
    receiveDescriptors[index].nextDescriptor = (uint32_t)
      &receiveDescriptors[(index + 1U) % ETHERNET_RX_DESCRIPTOR_COUNT];
  }

  for (uint32_t index = 0U; index < ETHERNET_TX_DESCRIPTOR_COUNT; index++) {
    transmitDescriptors[index].status = ETHERNET_TX_CHAINED;
    transmitDescriptors[index].control = 0U;
    transmitDescriptors[index].bufferAddress =
      (uint32_t)&transmitBuffers[index][0];
    transmitDescriptors[index].nextDescriptor = (uint32_t)
      &transmitDescriptors[(index + 1U) % ETHERNET_TX_DESCRIPTOR_COUNT];
  }

  ETH->DMARDLAR = (uint32_t)&receiveDescriptors[0];
  ETH->DMATDLAR = (uint32_t)&transmitDescriptors[0];

  status = ethernetMac_WritePhy(
    phyAddress,
    LAN8742_BCR,
    LAN8742_BCR_AUTONEGO_EN | LAN8742_BCR_RESTART_AUTONEGO
  );
  if (status != ETHERNET_STATUS_OK)
    return status;

  SET_BIT(ETH->MACCR, ETH_MACCR_DM | ETH_MACCR_FES | ETH_MACCR_IPCO);

  SET_BIT(ETH->DMAOMR, ETH_DMAOMR_SR | ETH_DMAOMR_ST);
  SET_BIT(ETH->MACCR, ETH_MACCR_RE | ETH_MACCR_TE);
  return ETHERNET_STATUS_OK;
}

Ethernet_StatusTypeDef EthernetMac_UpdateLink(uint8_t* linkUp) {
  if (linkUp == NULL)
    return ETHERNET_STATUS_INVALID_ARGUMENT;
  uint16_t basicStatus = 0U;
  Ethernet_StatusTypeDef status = ethernetMac_ReadPhy(
    ethernetPhyAddress, LAN8742_BSR, &basicStatus
  );
  if (status == ETHERNET_STATUS_OK) {
    status = ethernetMac_ReadPhy(
      ethernetPhyAddress, LAN8742_BSR, &basicStatus
    );
  }
  if (status != ETHERNET_STATUS_OK)
    return status;
  *linkUp = 0U;
  if ((basicStatus & LAN8742_BSR_LINK_STATUS) == 0U)
    return ETHERNET_STATUS_OK;
  status = ethernetMac_ConfigureLink(ethernetPhyAddress);
  if (status == ETHERNET_STATUS_PHY_NEGOTIATING)
    return ETHERNET_STATUS_OK;
  if (status == ETHERNET_STATUS_OK)
    *linkUp = 1U;
  return status;
}

Ethernet_StatusTypeDef EthernetMac_Transmit(
  const uint8_t* frame,
  size_t length
) {
  if ((frame == NULL) || (length == 0U))
    return ETHERNET_STATUS_INVALID_ARGUMENT;
  if (length > ETHERNET_BUFFER_SIZE)
    return ETHERNET_STATUS_FRAME_TOO_LARGE;
  EthernetDmaDescriptor_TypeDef* descriptor =
    &transmitDescriptors[transmitDescriptorIndex];
  if ((descriptor->status & ETHERNET_TX_OWN) != 0U)
    return ETHERNET_STATUS_TRANSMIT_BUSY;
  memcpy(transmitBuffers[transmitDescriptorIndex], frame, length);
  descriptor->control = (uint32_t)length;
  __DMB();
  descriptor->status = ETHERNET_TX_OWN | ETHERNET_TX_FIRST_SEGMENT
    | ETHERNET_TX_LAST_SEGMENT | ETHERNET_TX_CHECKSUM_FULL
    | ETHERNET_TX_CHAINED;
  transmitDescriptorIndex =
    (transmitDescriptorIndex + 1U) % ETHERNET_TX_DESCRIPTOR_COUNT;
  if ((ETH->DMASR & ETH_DMASR_TBUS) != 0U)
    ETH->DMASR = ETH_DMASR_TBUS;
  ETH->DMATPDR = 0U;
  return ETHERNET_STATUS_OK;
}

Ethernet_StatusTypeDef EthernetMac_GetReceivedFrame(
  const uint8_t** frame,
  size_t* length
) {
  if ((frame == NULL) || (length == NULL))
    return ETHERNET_STATUS_INVALID_ARGUMENT;
  EthernetDmaDescriptor_TypeDef* descriptor =
    &receiveDescriptors[receiveDescriptorIndex];
  uint32_t status = descriptor->status;
  if ((status & ETHERNET_RX_OWN) != 0U)
    return ETHERNET_STATUS_NO_FRAME;
  size_t frameLength = ETHERNET_RX_FRAME_LENGTH(status);
  if (((status & ETHERNET_RX_ERROR) != 0U)
      || ((status & ETHERNET_RX_FIRST_SEGMENT) == 0U)
      || ((status & ETHERNET_RX_LAST_SEGMENT) == 0U)
      || (frameLength < 4U)) {
    EthernetMac_ReleaseReceivedFrame();
    return ETHERNET_STATUS_FRAME_ERROR;
  }
  frameLength -= 4U;
  if (frameLength > ETHERNET_BUFFER_SIZE) {
    EthernetMac_ReleaseReceivedFrame();
    return ETHERNET_STATUS_FRAME_TOO_LARGE;
  }
  *frame = receiveBuffers[receiveDescriptorIndex];
  *length = frameLength;
  return ETHERNET_STATUS_OK;
}

void EthernetMac_ReleaseReceivedFrame(void) {
  EthernetDmaDescriptor_TypeDef* descriptor =
    &receiveDescriptors[receiveDescriptorIndex];
  descriptor->status = ETHERNET_RX_OWN;
  __DMB();
  receiveDescriptorIndex =
    (receiveDescriptorIndex + 1U) % ETHERNET_RX_DESCRIPTOR_COUNT;
  if ((ETH->DMASR & ETH_DMASR_RBUS) != 0U)
    ETH->DMASR = ETH_DMASR_RBUS;
  ETH->DMARPDR = 0U;
}

void EthernetMac_GetAddress(uint8_t macAddress[6]) {
  if (macAddress == NULL)
    return;
  uint32_t low = ETH->MACA0LR;
  uint32_t high = ETH->MACA0HR;
  macAddress[0] = (uint8_t)low;
  macAddress[1] = (uint8_t)(low >> 8U);
  macAddress[2] = (uint8_t)(low >> 16U);
  macAddress[3] = (uint8_t)(low >> 24U);
  macAddress[4] = (uint8_t)high;
  macAddress[5] = (uint8_t)(high >> 8U);
}

static Ethernet_StatusTypeDef ethernetMac_WaitForClear(
  volatile uint32_t* registerAddress,
  uint32_t mask,
  Ethernet_StatusTypeDef timeoutStatus
) {
  uint32_t remaining = ETHERNET_REGISTER_TIMEOUT;
  while ((READ_REG(*registerAddress) & mask) != 0U) {
    if (--remaining == 0U)
      return timeoutStatus;
  }
  return ETHERNET_STATUS_OK;
}

static Ethernet_StatusTypeDef ethernetMac_WritePhy(
  uint8_t phyAddress,
  uint8_t registerAddress,
  uint16_t value
) {
  Ethernet_StatusTypeDef status = ethernetMac_WaitForClear(
    &ETH->MACMIIAR,
    ETH_MACMIIAR_MB,
    ETHERNET_STATUS_MDIO_TIMEOUT
  );
  if (status != ETHERNET_STATUS_OK)
    return status;

  ETH->MACMIIDR = value;
  ETH->MACMIIAR = ETH_MACMIIAR_MB | ETH_MACMIIAR_MW
    | (0x4UL << ETH_MACMIIAR_CR_Pos)
    | ((uint32_t)registerAddress << ETH_MACMIIAR_MR_Pos)
    | ((uint32_t)phyAddress << ETH_MACMIIAR_PA_Pos);
  return ethernetMac_WaitForClear(
    &ETH->MACMIIAR,
    ETH_MACMIIAR_MB,
    ETHERNET_STATUS_MDIO_TIMEOUT
  );
}

static Ethernet_StatusTypeDef ethernetMac_ReadPhy(
  uint8_t phyAddress,
  uint8_t registerAddress,
  uint16_t* value
) {
  Ethernet_StatusTypeDef status = ethernetMac_WaitForClear(
    &ETH->MACMIIAR,
    ETH_MACMIIAR_MB,
    ETHERNET_STATUS_MDIO_TIMEOUT
  );
  if (status != ETHERNET_STATUS_OK)
    return status;

  ETH->MACMIIAR = ETH_MACMIIAR_MB
    | (0x4UL << ETH_MACMIIAR_CR_Pos)
    | ((uint32_t)registerAddress << ETH_MACMIIAR_MR_Pos)
    | ((uint32_t)phyAddress << ETH_MACMIIAR_PA_Pos);
  status = ethernetMac_WaitForClear(
    &ETH->MACMIIAR,
    ETH_MACMIIAR_MB,
    ETHERNET_STATUS_MDIO_TIMEOUT
  );
  if (status == ETHERNET_STATUS_OK)
    *value = (uint16_t)ETH->MACMIIDR;
  return status;
}

static Ethernet_StatusTypeDef ethernetMac_ConfigureLink(uint8_t phyAddress) {
  uint16_t phyStatus = 0U;
  Ethernet_StatusTypeDef status = ethernetMac_ReadPhy(
    phyAddress,
    LAN8742_PHYSCSR,
    &phyStatus
  );
  if (status != ETHERNET_STATUS_OK)
    return status;

  if ((phyStatus & LAN8742_PHYSCSR_AUTONEGO_DONE) == 0U)
    return ETHERNET_STATUS_PHY_NEGOTIATING;

  CLEAR_BIT(ETH->MACCR, ETH_MACCR_DM | ETH_MACCR_FES);
  switch (phyStatus & LAN8742_PHYSCSR_HCDSPEEDMASK) {
    case LAN8742_PHYSCSR_100BTX_FD:
      SET_BIT(ETH->MACCR, ETH_MACCR_DM | ETH_MACCR_FES);
      break;
    case LAN8742_PHYSCSR_100BTX_HD:
      SET_BIT(ETH->MACCR, ETH_MACCR_FES);
      break;
    case LAN8742_PHYSCSR_10BT_FD:
      SET_BIT(ETH->MACCR, ETH_MACCR_DM);
      break;
    case LAN8742_PHYSCSR_10BT_HD:
      break;
    default:
      return ETHERNET_STATUS_PHY_MODE_ERROR;
  }

  SET_BIT(ETH->MACCR, ETH_MACCR_IPCO);
  return ETHERNET_STATUS_OK;
}
