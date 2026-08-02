/**
  ******************************************************************************
  * @file           : tls_platform.c
  * @brief          : STM32 entropy, UTC, and bounded allocator for Mbed TLS.
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

#include "tls_platform.h"

#include "FreeRTOS.h"
#include "stm32f767xx.h"
#include "mbedtls/entropy.h"
#include "mbedtls/memory_buffer_alloc.h"
#include "mbedtls/platform.h"
#include "mbedtls/platform_time.h"
#include "psa/crypto.h"
#include "time_service.h"
#include "semphr.h"

#include <stdlib.h>
#include <string.h>

#define TLS_PLATFORM_HEAP_SIZE       (96U * 1024U)
#define TLS_PLATFORM_RNG_TIMEOUT_MS  100U

static uint8_t tlsHeap[TLS_PLATFORM_HEAP_SIZE] __attribute__((aligned(8)));
static StaticSemaphore_t cryptoMutexControlBlock;
static SemaphoreHandle_t cryptoMutex;

int mbedtls_hardware_poll(
  void* data,
  unsigned char* output,
  size_t length,
  size_t* outputLength
);

static HealthCheck_StatusTypeDef tlsPlatform_ReadRandom(uint32_t* value) {
  if (value == NULL)
    return HEALTH_CHECK_STATUS_ERROR;
  uint32_t started = TimeService_GetUptimeMs();
  while ((RNG->SR & RNG_SR_DRDY) == 0U) {
    if ((RNG->SR & (RNG_SR_CECS | RNG_SR_SECS)) != 0U)
      return HEALTH_CHECK_STATUS_ERROR;
    if ((TimeService_GetUptimeMs() - started) >= TLS_PLATFORM_RNG_TIMEOUT_MS)
      return HEALTH_CHECK_STATUS_ERROR;
  }
  *value = RNG->DR;
  return HEALTH_CHECK_STATUS_OK;
}

static mbedtls_time_t tlsPlatform_GetTime(mbedtls_time_t* currentTime) {
  uint32_t unixTime = 0U;
  if ((TimeService_IsSynchronized() == 0U)
      || (TimeService_GetUnixTime(&unixTime) != HEALTH_CHECK_STATUS_OK)) {
    return 0;
  }

  if (currentTime != NULL)
    *currentTime = (mbedtls_time_t)unixTime;
  return (mbedtls_time_t)unixTime;
}

HealthCheck_StatusTypeDef TlsPlatform_Init(void) {
  cryptoMutex = xSemaphoreCreateRecursiveMutexStatic(
    &cryptoMutexControlBlock
  );
  if (cryptoMutex == NULL)
    return HEALTH_CHECK_STATUS_ERROR;

  SET_BIT(RCC->AHB2ENR, RCC_AHB2ENR_RNGEN);
  (void)RCC->AHB2ENR;
  SET_BIT(RNG->CR, RNG_CR_RNGEN);

  uint32_t randomSeed;
  if (tlsPlatform_ReadRandom(&randomSeed) != HEALTH_CHECK_STATUS_OK) {
    return HEALTH_CHECK_STATUS_ERROR;
  }
  srand(randomSeed);

  mbedtls_memory_buffer_alloc_init(tlsHeap, sizeof(tlsHeap));
  if (mbedtls_platform_set_time(tlsPlatform_GetTime) != 0)
    return HEALTH_CHECK_STATUS_ERROR;
  if (psa_crypto_init() != PSA_SUCCESS)
    return HEALTH_CHECK_STATUS_ERROR;
  return HEALTH_CHECK_STATUS_OK;
}

HealthCheck_StatusTypeDef TlsPlatform_Lock(void) {
  if (cryptoMutex == NULL)
    return HEALTH_CHECK_STATUS_ERROR;
  return (xSemaphoreTakeRecursive(cryptoMutex, portMAX_DELAY) == pdTRUE)
    ? HEALTH_CHECK_STATUS_OK
    : HEALTH_CHECK_STATUS_ERROR;
}

void TlsPlatform_Unlock(void) {
  if (cryptoMutex != NULL)
    (void)xSemaphoreGiveRecursive(cryptoMutex);
}

HealthCheck_StatusTypeDef TlsPlatform_Random(uint8_t* output, size_t length) {
  if ((output == NULL) && (length != 0U))
    return HEALTH_CHECK_STATUS_ERROR;
  if (TlsPlatform_Lock() != HEALTH_CHECK_STATUS_OK)
    return HEALTH_CHECK_STATUS_ERROR;
  size_t generated = 0U;
  HealthCheck_StatusTypeDef status = (mbedtls_hardware_poll(
    NULL, output, length, &generated
  ) == 0) && (generated == length)
    ? HEALTH_CHECK_STATUS_OK
    : HEALTH_CHECK_STATUS_ERROR;
  TlsPlatform_Unlock();
  return status;
}

int mbedtls_hardware_poll(
  void* data,
  unsigned char* output,
  size_t length,
  size_t* outputLength
) {
  (void)data;
  if ((output == NULL) || (outputLength == NULL))
    return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;

  size_t offset = 0U;
  while (offset < length) {
    uint32_t randomValue;
    if (tlsPlatform_ReadRandom(&randomValue) != HEALTH_CHECK_STATUS_OK) {
      *outputLength = offset;
      return MBEDTLS_ERR_ENTROPY_SOURCE_FAILED;
    }

    size_t chunk = length - offset;
    if (chunk > sizeof(randomValue))
      chunk = sizeof(randomValue);
    memcpy(&output[offset], &randomValue, chunk);
    offset += chunk;
  }

  *outputLength = offset;
  return 0;
}

mbedtls_ms_time_t mbedtls_ms_time(void) {
  return (mbedtls_ms_time_t)TimeService_GetUptimeMs();
}
