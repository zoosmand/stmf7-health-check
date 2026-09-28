#include "callback_config.h"

#include "FreeRTOS.h"
#include "flash_layout.h"
#include "semphr.h"
#include "tls_trust_store.h"
#include "w25q64.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define CALLBACK_CONFIG_MAGIC   0x43424B43UL
#define CALLBACK_CONFIG_VERSION 1U

typedef struct {
  uint32_t magic;
  uint16_t version;
  uint16_t reserved;
  uint32_t generation;
  CallbackConfig_TypeDef config;
  uint32_t crc;
} CallbackConfig_SnapshotTypeDef;

_Static_assert(
  sizeof(CallbackConfig_SnapshotTypeDef) <= W25Q64_SECTOR_SIZE,
  "Callback configuration snapshot must fit in one NOR sector"
);

static CallbackConfig_SnapshotTypeDef snapshot;
static uint32_t activeAddress;
static StaticSemaphore_t configMutexControlBlock;
static SemaphoreHandle_t configMutex;

/** @brief Accept bounded DNS names without URI syntax or whitespace. */
static uint8_t callbackConfig_HostValid(const char* host) {
  if ((host == NULL) || (host[0] == '\0'))
    return 0U;
  for (size_t index = 0U; index < CALLBACK_CONFIG_HOST_SIZE; ++index) {
    char character = host[index];
    if (character == '\0')
      return 1U;
    if (!(((character >= 'a') && (character <= 'z'))
          || ((character >= 'A') && (character <= 'Z'))
          || ((character >= '0') && (character <= '9'))
          || (character == '.') || (character == '-'))) {
      return 0U;
    }
  }
  return 0U;
}

/** @brief Accept a bounded visible-ASCII origin-form request target. */
static uint8_t callbackConfig_PathValid(const char* path) {
  if ((path == NULL) || (path[0] != '/'))
    return 0U;
  for (size_t index = 0U; index < CALLBACK_CONFIG_PATH_SIZE; ++index) {
    uint8_t character = (uint8_t)path[index];
    if (character == '\0')
      return 1U;
    if ((character < 0x21U) || (character > 0x7EU)
        || (character == '"') || (character == '\\')
        || (character == '#')) {
      return 0U;
    }
  }
  return 0U;
}

/** @brief Calculate the snapshot integrity CRC-32. */
static uint32_t callbackConfig_Crc(const void* data, size_t length) {
  const uint8_t* bytes = data;
  uint32_t crc = 0xFFFFFFFFUL;
  while (length-- != 0U) {
    crc ^= *bytes++;
    for (uint8_t bit = 0U; bit < 8U; ++bit)
      crc = (crc >> 1U) ^ ((crc & 1U) ? 0xEDB88320UL : 0U);
  }
  return ~crc;
}

/** @brief Validate fields and the maximum fully encoded GET target. */
static uint8_t callbackConfig_FieldsValid(
  const CallbackConfig_TypeDef* config
) {
  if (!((config != NULL) && (config->enabled <= 1U)
      && (config->method <= CALLBACK_METHOD_POST) && (config->port != 0U)
      && (config->trustAnchorId >= TLS_TRUST_STORE_MIN_ID)
      && (config->trustAnchorId <= TLS_TRUST_STORE_MAX_PERSISTED)
      && (callbackConfig_HostValid(config->host) != 0U)
      && (callbackConfig_PathValid(config->path) != 0U))) {
    return 0U;
  }
  char resource[256];
  char separator = strchr(config->path, '?') == NULL ? '?' : '&';
  int length = snprintf(
    resource, sizeof(resource),
    "%s%csequence=4294967295&timestamp=4294967295&resource_index=255&"
    "status=fail&stage=certificate_error&http_status=65535&"
    "elapsed_ms=4294967295&detail=-2147483648",
    config->path, separator
  );
  return ((length > 0) && ((size_t)length < sizeof(resource))) ? 1U : 0U;
}

/** @brief Validate snapshot identity, fields, and integrity checksum. */
static uint8_t callbackConfig_IsValid(
  const CallbackConfig_SnapshotTypeDef* candidate
) {
  return ((candidate->magic == CALLBACK_CONFIG_MAGIC)
      && (candidate->version == CALLBACK_CONFIG_VERSION)
      && (callbackConfig_FieldsValid(&candidate->config) != 0U)
      && (candidate->crc == callbackConfig_Crc(
        candidate, offsetof(CallbackConfig_SnapshotTypeDef, crc)
      ))) ? 1U : 0U;
}

/** @brief Write and verify the inactive A/B sector before publishing it. */
static HealthCheck_StatusTypeDef callbackConfig_Save(
  CallbackConfig_SnapshotTypeDef* candidate
) {
  uint32_t target = activeAddress == FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_A
    ? FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_B
    : FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_A;
  candidate->magic = CALLBACK_CONFIG_MAGIC;
  candidate->version = CALLBACK_CONFIG_VERSION;
  ++candidate->generation;
  candidate->crc = callbackConfig_Crc(
    candidate, offsetof(CallbackConfig_SnapshotTypeDef, crc)
  );
  if ((W25Q64_EraseSector(target) != W25Q64_STATUS_OK)
      || (W25Q64_Program(target, candidate, sizeof(*candidate))
          != W25Q64_STATUS_OK)) {
    return HEALTH_CHECK_STATUS_ERROR;
  }
  static CallbackConfig_SnapshotTypeDef verification;
  if ((W25Q64_Read(target, &verification, sizeof(verification))
        != W25Q64_STATUS_OK)
      || (callbackConfig_IsValid(&verification) == 0U)
      || (verification.generation != candidate->generation)) {
    return HEALTH_CHECK_STATUS_ERROR;
  }
  snapshot = *candidate;
  activeAddress = target;
  return HEALTH_CHECK_STATUS_OK;
}

HealthCheck_StatusTypeDef CallbackConfig_Init(void) {
  configMutex = xSemaphoreCreateMutexStatic(&configMutexControlBlock);
  if (configMutex == NULL)
    return HEALTH_CHECK_STATUS_ERROR;
  static CallbackConfig_SnapshotTypeDef first;
  static CallbackConfig_SnapshotTypeDef second;
  uint8_t firstValid = (W25Q64_Read(
    FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_A, &first, sizeof(first)
  ) == W25Q64_STATUS_OK) && callbackConfig_IsValid(&first);
  uint8_t secondValid = (W25Q64_Read(
    FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_B, &second, sizeof(second)
  ) == W25Q64_STATUS_OK) && callbackConfig_IsValid(&second);
  if ((firstValid != 0U)
      && ((secondValid == 0U) || (first.generation >= second.generation))) {
    snapshot = first;
    activeAddress = FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_A;
    return HEALTH_CHECK_STATUS_OK;
  }
  if (secondValid != 0U) {
    snapshot = second;
    activeAddress = FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_B;
    return HEALTH_CHECK_STATUS_OK;
  }
  memset(&snapshot, 0, sizeof(snapshot));
  snapshot.config.method = CALLBACK_METHOD_POST;
  snapshot.config.port = 443U;
  snapshot.config.trustAnchorId = TLS_TRUST_STORE_MIN_ID;
  (void)strncpy(
    snapshot.config.host, "loopback.intraclear.com",
    sizeof(snapshot.config.host) - 1U
  );
  (void)strncpy(
    snapshot.config.path, "/", sizeof(snapshot.config.path) - 1U
  );
  activeAddress = FLASH_LAYOUT_CALLBACK_CONFIG_SECTOR_B;
  return callbackConfig_Save(&snapshot);
}

void CallbackConfig_Get(CallbackConfig_TypeDef* config) {
  if (config == NULL)
    return;
  memset(config, 0, sizeof(*config));
  if (xSemaphoreTake(configMutex, portMAX_DELAY) == pdTRUE) {
    *config = snapshot.config;
    (void)xSemaphoreGive(configMutex);
  }
}

HealthCheck_StatusTypeDef CallbackConfig_Set(
  const CallbackConfig_TypeDef* config
) {
  if (callbackConfig_FieldsValid(config) == 0U)
    return HEALTH_CHECK_STATUS_ERROR;
  if (xSemaphoreTake(configMutex, portMAX_DELAY) != pdTRUE)
    return HEALTH_CHECK_STATUS_ERROR;
  CallbackConfig_SnapshotTypeDef candidate = snapshot;
  candidate.config = *config;
  HealthCheck_StatusTypeDef status = callbackConfig_Save(&candidate);
  (void)xSemaphoreGive(configMutex);
  return status;
}

uint8_t CallbackConfig_IsTrustAnchorInUse(uint8_t trustAnchorId) {
  CallbackConfig_TypeDef config = {0};
  CallbackConfig_Get(&config);
  return ((config.enabled != 0U) && (config.trustAnchorId == trustAnchorId))
    ? 1U : 0U;
}
