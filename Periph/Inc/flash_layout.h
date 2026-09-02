/**
  ******************************************************************************
  * @file           : flash_layout.h
  * @brief          : Centralized W25Q64 sector allocation for persistent stores.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 01.08.2026
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

#ifndef FLASH_LAYOUT_H
#define FLASH_LAYOUT_H

#include "w25q64.h"

/**
  * Sectors are allocated from the top of the W25Q64's address space downward.
  * Every store owns a fixed, non-overlapping range. Add new stores by
  * extending this list; never resize or reorder an existing entry without a
  * migration plan, since existing on-flash data lives at these addresses.
  */

#define FLASH_LAYOUT_USER_STORE_SECTOR_A \
  (W25Q64_CAPACITY_BYTES - (2U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_USER_STORE_SECTOR_B \
  (W25Q64_CAPACITY_BYTES - (1U * W25Q64_SECTOR_SIZE))

#define FLASH_LAYOUT_HEALTH_CHECK_CONFIG_SECTOR_A \
  (W25Q64_CAPACITY_BYTES - (4U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_HEALTH_CHECK_CONFIG_SECTOR_B \
  (W25Q64_CAPACITY_BYTES - (3U * W25Q64_SECTOR_SIZE))

#define FLASH_LAYOUT_TLS_SERVER_CREDENTIALS_SECTOR_A \
  (W25Q64_CAPACITY_BYTES - (6U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_TLS_SERVER_CREDENTIALS_SECTOR_B \
  (W25Q64_CAPACITY_BYTES - (5U * W25Q64_SECTOR_SIZE))

#define FLASH_LAYOUT_HEALTH_CHECK_LOG_SECTOR_0 \
  (W25Q64_CAPACITY_BYTES - (8U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_HEALTH_CHECK_LOG_SECTOR_1 \
  (W25Q64_CAPACITY_BYTES - (7U * W25Q64_SECTOR_SIZE))

#define FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_A \
  (W25Q64_CAPACITY_BYTES - (12U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_B \
  (W25Q64_CAPACITY_BYTES - (10U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_SECTORS 2U

#define FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A \
  (W25Q64_CAPACITY_BYTES - (26U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B \
  (W25Q64_CAPACITY_BYTES - (22U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_TLS_TRUST_STORE_BANK_SECTORS 4U

/* Factory-reset marker is kept outside the range erased by a reset. */
#define FLASH_LAYOUT_FACTORY_RESET_MARKER_SECTOR \
  (W25Q64_CAPACITY_BYTES - (27U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_FACTORY_RESET_DATA_START \
  (W25Q64_CAPACITY_BYTES - (26U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_FACTORY_RESET_DATA_LENGTH \
  (26U * W25Q64_SECTOR_SIZE)

#define FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_A \
  (W25Q64_CAPACITY_BYTES - (18U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_B \
  (W25Q64_CAPACITY_BYTES - (15U * W25Q64_SECTOR_SIZE))
#define FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_SECTORS 3U

_Static_assert(
  FLASH_LAYOUT_FACTORY_RESET_MARKER_SECTOR + W25Q64_SECTOR_SIZE
    <= FLASH_LAYOUT_FACTORY_RESET_DATA_START,
  "Factory-reset marker overlaps persistent data"
);
_Static_assert(
  FLASH_LAYOUT_FACTORY_RESET_DATA_START + FLASH_LAYOUT_FACTORY_RESET_DATA_LENGTH
    == W25Q64_CAPACITY_BYTES,
  "Factory-reset range must end at Flash capacity"
);
_Static_assert(
  FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A
    + (FLASH_LAYOUT_TLS_TRUST_STORE_BANK_SECTORS * W25Q64_SECTOR_SIZE)
    <= FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B,
  "TLS trust-store banks overlap"
);
_Static_assert(
  FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B
    + (FLASH_LAYOUT_TLS_TRUST_STORE_BANK_SECTORS * W25Q64_SECTOR_SIZE)
    <= FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_A,
  "TLS trust store overlaps its previous banks"
);
_Static_assert(
  FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_A
    + (FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_SECTORS
      * W25Q64_SECTOR_SIZE)
    <= FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_B,
  "Previous TLS trust-store banks overlap"
);
_Static_assert(
  FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_B
    + (FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_SECTORS
      * W25Q64_SECTOR_SIZE)
    <= FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_A,
  "Previous TLS trust store overlaps its legacy banks"
);
_Static_assert(
  FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_A
    + (FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_SECTORS * W25Q64_SECTOR_SIZE)
    <= FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_B,
  "Legacy TLS trust-store banks overlap"
);
_Static_assert(
  FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_B
    + (FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_SECTORS * W25Q64_SECTOR_SIZE)
    <= FLASH_LAYOUT_HEALTH_CHECK_LOG_SECTOR_0,
  "Legacy TLS trust store overlaps the health-check log"
);

#endif /* FLASH_LAYOUT_H */
