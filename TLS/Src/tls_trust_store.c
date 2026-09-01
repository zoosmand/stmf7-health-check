/**
  ******************************************************************************
  * @file           : tls_trust_store.c
  * @brief          : Root certificates trusted by the HTTPS transport.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 30.07.2026
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

#include "tls_trust_store.h"

#include "FreeRTOS.h"
#include "flash_layout.h"
#include "semphr.h"
#include "w25q64.h"

#include "mbedtls/asn1.h"
#include "mbedtls/bignum.h"
#include "mbedtls/pk.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define TLS_TRUST_STORE_MAGIC   0x54525354UL
#define TLS_TRUST_STORE_VERSION 3U
#define TLS_TRUST_STORE_PREVIOUS_VERSION 2U
#define TLS_TRUST_STORE_PREVIOUS_MAX_PERSISTED 5U
#define TLS_TRUST_STORE_LEGACY_VERSION 1U
#define TLS_TRUST_STORE_LEGACY_MAX_PERSISTED 3U

typedef struct {
  uint8_t occupied;
  uint8_t reserved;
  uint16_t derLength;
  char subject[TLS_TRUST_STORE_SUBJECT_SIZE];
  uint8_t der[TLS_TRUST_STORE_MAX_DER_SIZE];
} TlsTrustStore_AnchorTypeDef;

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t generation;
  TlsTrustStore_AnchorTypeDef anchors[TLS_TRUST_STORE_MAX_PERSISTED];
  uint32_t crc;
} TlsTrustStore_SnapshotTypeDef;

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t generation;
  TlsTrustStore_AnchorTypeDef anchors[
    TLS_TRUST_STORE_PREVIOUS_MAX_PERSISTED
  ];
  uint32_t crc;
} TlsTrustStore_PreviousSnapshotTypeDef;

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t generation;
  TlsTrustStore_AnchorTypeDef anchors[
    TLS_TRUST_STORE_LEGACY_MAX_PERSISTED
  ];
  uint32_t crc;
} TlsTrustStore_LegacySnapshotTypeDef;

_Static_assert(
  sizeof(TlsTrustStore_SnapshotTypeDef)
    <= (FLASH_LAYOUT_TLS_TRUST_STORE_BANK_SECTORS * W25Q64_SECTOR_SIZE),
  "TLS trust store exceeds its Flash bank"
);

_Static_assert(
  sizeof(TlsTrustStore_PreviousSnapshotTypeDef)
    <= (FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_SECTORS
      * W25Q64_SECTOR_SIZE),
  "Previous TLS trust store exceeds its Flash bank"
);

_Static_assert(
  sizeof(TlsTrustStore_LegacySnapshotTypeDef)
    <= (FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_SECTORS
      * W25Q64_SECTOR_SIZE),
  "Legacy TLS trust store exceeds its Flash bank"
);

static TlsTrustStore_SnapshotTypeDef trustStoreSnapshot;
static union {
  TlsTrustStore_SnapshotTypeDef current;
  TlsTrustStore_PreviousSnapshotTypeDef previous;
  TlsTrustStore_LegacySnapshotTypeDef legacy;
} trustStoreWorkspace;
#define trustStoreCandidate trustStoreWorkspace.current
static uint32_t trustStoreActiveAddress;
static StaticSemaphore_t trustStoreMutexControlBlock;
static SemaphoreHandle_t trustStoreMutex;

static uint32_t tlsTrustStore_Crc(const void* data, size_t length) {
  const uint8_t* bytes = data;
  uint32_t crc = 0xFFFFFFFFUL;
  while (length-- != 0U) {
    crc ^= *bytes++;
    for (uint8_t bit = 0U; bit < 8U; ++bit)
      crc = (crc >> 1U) ^ ((crc & 1U) ? 0xEDB88320UL : 0U);
  }
  return ~crc;
}

static uint8_t tlsTrustStore_IsSnapshotValid(
  const TlsTrustStore_SnapshotTypeDef* candidate
) {
  if ((candidate->magic != TLS_TRUST_STORE_MAGIC)
      || (candidate->version != TLS_TRUST_STORE_VERSION)
      || (candidate->crc != tlsTrustStore_Crc(
        candidate, offsetof(TlsTrustStore_SnapshotTypeDef, crc)
      ))) {
    return 0U;
  }
  for (uint8_t index = 0U; index < TLS_TRUST_STORE_MAX_PERSISTED; ++index) {
    const TlsTrustStore_AnchorTypeDef* anchor = &candidate->anchors[index];
    if ((anchor->occupied != 0U)
        && ((anchor->derLength == 0U)
          || (anchor->derLength > TLS_TRUST_STORE_MAX_DER_SIZE)
          || (memchr(anchor->subject, '\0', sizeof(anchor->subject)) == NULL))) {
      return 0U;
    }
  }
  return 1U;
}

static uint8_t tlsTrustStore_IsLegacySnapshotValid(
  const TlsTrustStore_LegacySnapshotTypeDef* candidate
) {
  if ((candidate->magic != TLS_TRUST_STORE_MAGIC)
      || (candidate->version != TLS_TRUST_STORE_LEGACY_VERSION)
      || (candidate->crc != tlsTrustStore_Crc(
        candidate, offsetof(TlsTrustStore_LegacySnapshotTypeDef, crc)
      ))) {
    return 0U;
  }
  for (uint8_t index = 0U;
       index < TLS_TRUST_STORE_LEGACY_MAX_PERSISTED;
       ++index) {
    const TlsTrustStore_AnchorTypeDef* anchor = &candidate->anchors[index];
    if ((anchor->occupied != 0U)
        && ((anchor->derLength == 0U)
          || (anchor->derLength > TLS_TRUST_STORE_MAX_DER_SIZE)
          || (memchr(anchor->subject, '\0', sizeof(anchor->subject)) == NULL))) {
      return 0U;
    }
  }
  return 1U;
}

static uint8_t tlsTrustStore_IsPreviousSnapshotValid(
  const TlsTrustStore_PreviousSnapshotTypeDef* candidate
) {
  if ((candidate->magic != TLS_TRUST_STORE_MAGIC)
      || (candidate->version != TLS_TRUST_STORE_PREVIOUS_VERSION)
      || (candidate->crc != tlsTrustStore_Crc(
        candidate, offsetof(TlsTrustStore_PreviousSnapshotTypeDef, crc)
      ))) {
    return 0U;
  }
  for (uint8_t index = 0U;
       index < TLS_TRUST_STORE_PREVIOUS_MAX_PERSISTED;
       ++index) {
    const TlsTrustStore_AnchorTypeDef* anchor = &candidate->anchors[index];
    if ((anchor->occupied != 0U)
        && ((anchor->derLength == 0U)
          || (anchor->derLength > TLS_TRUST_STORE_MAX_DER_SIZE)
          || (memchr(anchor->subject, '\0', sizeof(anchor->subject)) == NULL))) {
      return 0U;
    }
  }
  return 1U;
}

static HealthCheck_StatusTypeDef tlsTrustStore_ReadSnapshot(
  uint32_t address,
  TlsTrustStore_SnapshotTypeDef* target
) {
  if (W25Q64_Read(address, target, sizeof(*target)) != W25Q64_STATUS_OK)
    return HEALTH_CHECK_STATUS_ERROR;
  return tlsTrustStore_IsSnapshotValid(target) != 0U ? HEALTH_CHECK_STATUS_OK : HEALTH_CHECK_STATUS_ERROR;
}

static HealthCheck_StatusTypeDef tlsTrustStore_ReadLegacySnapshot(
  uint32_t address,
  TlsTrustStore_LegacySnapshotTypeDef* target
) {
  if (W25Q64_Read(address, target, sizeof(*target)) != W25Q64_STATUS_OK)
    return HEALTH_CHECK_STATUS_ERROR;
  return tlsTrustStore_IsLegacySnapshotValid(target) != 0U
    ? HEALTH_CHECK_STATUS_OK
    : HEALTH_CHECK_STATUS_ERROR;
}

static HealthCheck_StatusTypeDef tlsTrustStore_ReadPreviousSnapshot(
  uint32_t address,
  TlsTrustStore_PreviousSnapshotTypeDef* target
) {
  if (W25Q64_Read(address, target, sizeof(*target)) != W25Q64_STATUS_OK)
    return HEALTH_CHECK_STATUS_ERROR;
  return tlsTrustStore_IsPreviousSnapshotValid(target) != 0U
    ? HEALTH_CHECK_STATUS_OK
    : HEALTH_CHECK_STATUS_ERROR;
}

static void tlsTrustStore_MigratePrevious(
  const TlsTrustStore_PreviousSnapshotTypeDef* previous,
  TlsTrustStore_SnapshotTypeDef* target
) {
  memset(target, 0, sizeof(*target));
  target->generation = previous->generation;
  memcpy(target->anchors, previous->anchors, sizeof(previous->anchors));
}

static void tlsTrustStore_MigrateLegacy(
  const TlsTrustStore_LegacySnapshotTypeDef* legacy,
  TlsTrustStore_SnapshotTypeDef* target
) {
  memset(target, 0, sizeof(*target));
  target->generation = legacy->generation;
  memcpy(target->anchors, legacy->anchors, sizeof(legacy->anchors));
}

static HealthCheck_StatusTypeDef tlsTrustStore_Save(
  const TlsTrustStore_SnapshotTypeDef* candidate
) {
  uint32_t target =
    (trustStoreActiveAddress == FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A)
      ? FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B
      : FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A;
  for (uint8_t sector = 0U;
       sector < FLASH_LAYOUT_TLS_TRUST_STORE_BANK_SECTORS;
       ++sector) {
    if (W25Q64_EraseSector(
          target + ((uint32_t)sector * W25Q64_SECTOR_SIZE)
        ) != W25Q64_STATUS_OK) {
      return HEALTH_CHECK_STATUS_ERROR;
    }
  }
  if (W25Q64_Program(target, candidate, sizeof(*candidate)) != W25Q64_STATUS_OK)
    return HEALTH_CHECK_STATUS_ERROR;
  if (tlsTrustStore_ReadSnapshot(target, &trustStoreCandidate) != HEALTH_CHECK_STATUS_OK)
    return HEALTH_CHECK_STATUS_ERROR;
  trustStoreSnapshot = trustStoreCandidate;
  trustStoreActiveAddress = target;
  return HEALTH_CHECK_STATUS_OK;
}

static TlsTrustStore_StatusTypeDef tlsTrustStore_CommitCandidate(void) {
  trustStoreCandidate.magic = TLS_TRUST_STORE_MAGIC;
  trustStoreCandidate.version = TLS_TRUST_STORE_VERSION;
  trustStoreCandidate.reserved = 0U;
  ++trustStoreCandidate.generation;
  trustStoreCandidate.crc = tlsTrustStore_Crc(
    &trustStoreCandidate, offsetof(TlsTrustStore_SnapshotTypeDef, crc)
  );
  return (tlsTrustStore_Save(&trustStoreCandidate) == HEALTH_CHECK_STATUS_OK)
    ? TLS_TRUST_STORE_STATUS_OK
    : TLS_TRUST_STORE_STATUS_STORAGE_ERROR;
}

static TlsTrustStore_StatusTypeDef tlsTrustStore_ValidateDer(
  const uint8_t* der,
  size_t length,
  TlsTrustStore_AnchorTypeDef* anchor
) {
  if ((der == NULL) || (length == 0U)
      || (length > TLS_TRUST_STORE_MAX_DER_SIZE) || (anchor == NULL)) {
    return TLS_TRUST_STORE_STATUS_INVALID_ARGUMENT;
  }
  mbedtls_x509_crt certificate;
  mbedtls_x509_crt_init(&certificate);
  int result = mbedtls_x509_crt_parse_der_nocopy(
    &certificate, der, length
  );
  if (result != 0) {
    mbedtls_x509_crt_free(&certificate);
    Common_Printf("Trust anchor parse failed: %d\r\n", result);
    if ((result == MBEDTLS_ERR_X509_ALLOC_FAILED)
        || (result == MBEDTLS_ERR_PK_ALLOC_FAILED)
        || ((-result & 0x007FU) == -MBEDTLS_ERR_MPI_ALLOC_FAILED)
        || ((-result & 0x007FU) == -MBEDTLS_ERR_ASN1_ALLOC_FAILED)) {
      return TLS_TRUST_STORE_STATUS_NO_MEMORY;
    }
    return TLS_TRUST_STORE_STATUS_INVALID_CERTIFICATE;
  }
  if ((mbedtls_x509_crt_get_ca_istrue(&certificate) != 1)
      || (mbedtls_x509_crt_check_key_usage(
        &certificate, MBEDTLS_X509_KU_KEY_CERT_SIGN
      ) != 0)) {
    mbedtls_x509_crt_free(&certificate);
    return TLS_TRUST_STORE_STATUS_NOT_CA;
  }
  memset(anchor, 0, sizeof(*anchor));
  int subjectLength = mbedtls_x509_dn_gets(
    anchor->subject, sizeof(anchor->subject), &certificate.subject
  );
  if ((subjectLength <= 0)
      || ((size_t)subjectLength >= sizeof(anchor->subject))) {
    mbedtls_x509_crt_free(&certificate);
    return TLS_TRUST_STORE_STATUS_INVALID_CERTIFICATE;
  }
  anchor->occupied = 1U;
  anchor->derLength = (uint16_t)length;
  memcpy(anchor->der, der, length);
  mbedtls_x509_crt_free(&certificate);
  return TLS_TRUST_STORE_STATUS_OK;
}

HealthCheck_StatusTypeDef TlsTrustStore_Init(void) {
  trustStoreMutex = xSemaphoreCreateMutexStatic(
    &trustStoreMutexControlBlock
  );
  if (trustStoreMutex == NULL)
    return HEALTH_CHECK_STATUS_ERROR;

  uint8_t firstValid = (tlsTrustStore_ReadSnapshot(
    FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A, &trustStoreSnapshot
  ) == HEALTH_CHECK_STATUS_OK);
  uint32_t firstGeneration = trustStoreSnapshot.generation;
  uint8_t secondValid = (tlsTrustStore_ReadSnapshot(
    FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B, &trustStoreCandidate
  ) == HEALTH_CHECK_STATUS_OK);

  if ((firstValid != 0U) && ((secondValid == 0U)
      || (firstGeneration >= trustStoreCandidate.generation))) {
    if (tlsTrustStore_ReadSnapshot(
          FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A, &trustStoreSnapshot
        ) != HEALTH_CHECK_STATUS_OK) {
      return HEALTH_CHECK_STATUS_ERROR;
    }
    trustStoreActiveAddress = FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A;
    return HEALTH_CHECK_STATUS_OK;
  }
  if (secondValid != 0U) {
    trustStoreSnapshot = trustStoreCandidate;
    trustStoreActiveAddress = FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B;
    return HEALTH_CHECK_STATUS_OK;
  }

  uint8_t previousValid = 0U;
  uint32_t previousGeneration = 0U;
  if (tlsTrustStore_ReadPreviousSnapshot(
        FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_A,
        &trustStoreWorkspace.previous
      ) == HEALTH_CHECK_STATUS_OK) {
    previousGeneration = trustStoreWorkspace.previous.generation;
    tlsTrustStore_MigratePrevious(
      &trustStoreWorkspace.previous, &trustStoreSnapshot
    );
    previousValid = 1U;
  }
  if (tlsTrustStore_ReadPreviousSnapshot(
        FLASH_LAYOUT_TLS_TRUST_STORE_PREVIOUS_BANK_B,
        &trustStoreWorkspace.previous
      ) == HEALTH_CHECK_STATUS_OK) {
    if ((previousValid == 0U)
        || (trustStoreWorkspace.previous.generation >= previousGeneration)) {
      tlsTrustStore_MigratePrevious(
        &trustStoreWorkspace.previous, &trustStoreSnapshot
      );
    }
    previousValid = 1U;
  }
  if (previousValid != 0U) {
    trustStoreCandidate = trustStoreSnapshot;
    trustStoreActiveAddress = FLASH_LAYOUT_TLS_TRUST_STORE_BANK_A;
    return tlsTrustStore_CommitCandidate() == TLS_TRUST_STORE_STATUS_OK
      ? HEALTH_CHECK_STATUS_OK
      : HEALTH_CHECK_STATUS_ERROR;
  }

  uint8_t legacyValid = 0U;
  uint32_t legacyGeneration = 0U;
  if (tlsTrustStore_ReadLegacySnapshot(
        FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_A,
        &trustStoreWorkspace.legacy
      ) == HEALTH_CHECK_STATUS_OK) {
    tlsTrustStore_MigrateLegacy(
      &trustStoreWorkspace.legacy, &trustStoreSnapshot
    );
    legacyGeneration = trustStoreWorkspace.legacy.generation;
    legacyValid = 1U;
  }
  if (tlsTrustStore_ReadLegacySnapshot(
        FLASH_LAYOUT_TLS_TRUST_STORE_LEGACY_BANK_B,
        &trustStoreWorkspace.legacy
      ) == HEALTH_CHECK_STATUS_OK) {
    if ((legacyValid == 0U)
        || (trustStoreWorkspace.legacy.generation >= legacyGeneration)) {
      tlsTrustStore_MigrateLegacy(
        &trustStoreWorkspace.legacy, &trustStoreSnapshot
      );
    }
    legacyValid = 1U;
  }
  if (legacyValid != 0U) {
    trustStoreCandidate = trustStoreSnapshot;
    trustStoreActiveAddress = FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B;
    return tlsTrustStore_CommitCandidate() == TLS_TRUST_STORE_STATUS_OK
      ? HEALTH_CHECK_STATUS_OK
      : HEALTH_CHECK_STATUS_ERROR;
  }

  memset(&trustStoreCandidate, 0, sizeof(trustStoreCandidate));
  trustStoreActiveAddress = FLASH_LAYOUT_TLS_TRUST_STORE_BANK_B;
  return tlsTrustStore_CommitCandidate() == TLS_TRUST_STORE_STATUS_OK
    ? HEALTH_CHECK_STATUS_OK
    : HEALTH_CHECK_STATUS_ERROR;
}

uint8_t TlsTrustStore_Exists(uint8_t id) {
  if ((id < TLS_TRUST_STORE_MIN_ID)
      || (id > TLS_TRUST_STORE_MAX_PERSISTED)
      || (trustStoreMutex == NULL))
    return 0U;
  uint8_t exists = 0U;
  if (xSemaphoreTake(trustStoreMutex, portMAX_DELAY) == pdTRUE) {
    exists = trustStoreSnapshot.anchors[id - 1U].occupied != 0U;
    (void)xSemaphoreGive(trustStoreMutex);
  }
  return exists;
}

size_t TlsTrustStore_List(
  TlsTrustStore_InfoTypeDef* anchors,
  size_t capacity
) {
  if ((anchors == NULL) || (capacity == 0U) || (trustStoreMutex == NULL))
    return 0U;
  size_t count = 0U;
  if (xSemaphoreTake(trustStoreMutex, portMAX_DELAY) != pdTRUE)
    return count;
  for (uint8_t index = 0U;
       (index < TLS_TRUST_STORE_MAX_PERSISTED) && (count < capacity);
       ++index) {
    const TlsTrustStore_AnchorTypeDef* anchor =
      &trustStoreSnapshot.anchors[index];
    if (anchor->occupied == 0U)
      continue;
    anchors[count].id = index + 1U;
    anchors[count].derLength = anchor->derLength;
    (void)strncpy(
      anchors[count].subject,
      anchor->subject,
      sizeof(anchors[count].subject) - 1U
    );
    anchors[count].subject[sizeof(anchors[count].subject) - 1U] = '\0';
    ++count;
  }
  (void)xSemaphoreGive(trustStoreMutex);
  return count;
}

TlsTrustStore_StatusTypeDef TlsTrustStore_Parse(
  uint8_t id,
  mbedtls_x509_crt* certificate
) {
  if (certificate == NULL)
    return TLS_TRUST_STORE_STATUS_INVALID_ARGUMENT;
  if ((id < TLS_TRUST_STORE_MIN_ID)
      || (id > TLS_TRUST_STORE_MAX_PERSISTED)
      || (trustStoreMutex == NULL))
    return TLS_TRUST_STORE_STATUS_NOT_FOUND;
  if (xSemaphoreTake(trustStoreMutex, portMAX_DELAY) != pdTRUE)
    return TLS_TRUST_STORE_STATUS_STORAGE_ERROR;
  const TlsTrustStore_AnchorTypeDef* anchor =
    &trustStoreSnapshot.anchors[id - 1U];
  TlsTrustStore_StatusTypeDef status = TLS_TRUST_STORE_STATUS_NOT_FOUND;
  if (anchor->occupied != 0U) {
    status = (mbedtls_x509_crt_parse_der(
      certificate, anchor->der, anchor->derLength
    ) == 0)
      ? TLS_TRUST_STORE_STATUS_OK
      : TLS_TRUST_STORE_STATUS_INVALID_CERTIFICATE;
  }
  (void)xSemaphoreGive(trustStoreMutex);
  return status;
}

TlsTrustStore_StatusTypeDef TlsTrustStore_Add(
  const uint8_t* der,
  size_t length,
  uint8_t* assignedId
) {
  if (trustStoreMutex == NULL)
    return TLS_TRUST_STORE_STATUS_STORAGE_ERROR;
  if (xSemaphoreTake(trustStoreMutex, portMAX_DELAY) != pdTRUE)
    return TLS_TRUST_STORE_STATUS_STORAGE_ERROR;
  uint8_t index;
  for (index = 0U; index < TLS_TRUST_STORE_MAX_PERSISTED; ++index) {
    if (trustStoreSnapshot.anchors[index].occupied == 0U)
      break;
  }
  if (index == TLS_TRUST_STORE_MAX_PERSISTED) {
    (void)xSemaphoreGive(trustStoreMutex);
    return TLS_TRUST_STORE_STATUS_FULL;
  }
  trustStoreCandidate = trustStoreSnapshot;
  TlsTrustStore_StatusTypeDef status = tlsTrustStore_ValidateDer(
    der, length, &trustStoreCandidate.anchors[index]
  );
  if (status == TLS_TRUST_STORE_STATUS_OK)
    status = tlsTrustStore_CommitCandidate();
  if ((status == TLS_TRUST_STORE_STATUS_OK) && (assignedId != NULL))
    *assignedId = index + 1U;
  (void)xSemaphoreGive(trustStoreMutex);
  return status;
}

TlsTrustStore_StatusTypeDef TlsTrustStore_Replace(
  uint8_t id,
  const uint8_t* der,
  size_t length
) {
  if ((id < TLS_TRUST_STORE_MIN_ID)
      || (id > TLS_TRUST_STORE_MAX_PERSISTED)
      || (trustStoreMutex == NULL))
    return TLS_TRUST_STORE_STATUS_NOT_FOUND;
  if (xSemaphoreTake(trustStoreMutex, portMAX_DELAY) != pdTRUE)
    return TLS_TRUST_STORE_STATUS_STORAGE_ERROR;
  if (trustStoreSnapshot.anchors[id - 1U].occupied == 0U) {
    (void)xSemaphoreGive(trustStoreMutex);
    return TLS_TRUST_STORE_STATUS_NOT_FOUND;
  }
  trustStoreCandidate = trustStoreSnapshot;
  TlsTrustStore_StatusTypeDef status = tlsTrustStore_ValidateDer(
    der, length, &trustStoreCandidate.anchors[id - 1U]
  );
  if (status == TLS_TRUST_STORE_STATUS_OK)
    status = tlsTrustStore_CommitCandidate();
  (void)xSemaphoreGive(trustStoreMutex);
  return status;
}

TlsTrustStore_StatusTypeDef TlsTrustStore_Delete(uint8_t id) {
  if ((id < TLS_TRUST_STORE_MIN_ID)
      || (id > TLS_TRUST_STORE_MAX_PERSISTED)
      || (trustStoreMutex == NULL))
    return TLS_TRUST_STORE_STATUS_NOT_FOUND;
  if (xSemaphoreTake(trustStoreMutex, portMAX_DELAY) != pdTRUE)
    return TLS_TRUST_STORE_STATUS_STORAGE_ERROR;
  if (trustStoreSnapshot.anchors[id - 1U].occupied == 0U) {
    (void)xSemaphoreGive(trustStoreMutex);
    return TLS_TRUST_STORE_STATUS_NOT_FOUND;
  }
  trustStoreCandidate = trustStoreSnapshot;
  memset(
    &trustStoreCandidate.anchors[id - 1U],
    0,
    sizeof(trustStoreCandidate.anchors[id - 1U])
  );
  TlsTrustStore_StatusTypeDef status = tlsTrustStore_CommitCandidate();
  (void)xSemaphoreGive(trustStoreMutex);
  return status;
}

TlsTrustStore_StatusTypeDef TlsTrustStore_Reset(void) {
  if ((trustStoreMutex == NULL)
      || (xSemaphoreTake(trustStoreMutex, portMAX_DELAY) != pdTRUE)) {
    return TLS_TRUST_STORE_STATUS_STORAGE_ERROR;
  }
  memset(&trustStoreCandidate, 0, sizeof(trustStoreCandidate));
  trustStoreCandidate.generation = trustStoreSnapshot.generation;
  TlsTrustStore_StatusTypeDef status = tlsTrustStore_CommitCandidate();
  (void)xSemaphoreGive(trustStoreMutex);
  return status;
}
