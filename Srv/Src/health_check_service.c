/**
  ******************************************************************************
  * @file           : health_check_service.c
  * @brief          : Periodic HTTPS resource health-check service.
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

#include "health_check_service.h"

#include "FreeRTOS.h"
#include "api_service.h"
#include "buzzer_service.h"
#include "health_check_config.h"
#include "health_check_log.h"
#include "common.h"
#include "network_service.h"
#include "time_service.h"
#include "task.h"
#include "tls_transport.h"
#include "tls_platform.h"
#include "tls_server_credentials.h"
#include "tls_trust_store.h"

#include <stdio.h>
#include <time.h>

#define HEALTH_CHECK_TASK_STACK_DEPTH 2048U
#define HEALTH_CHECK_WAIT_MS          1000U
#define HEALTH_CHECK_OK_STATUS        200U

static StaticTask_t healthCheckTaskControlBlock;
static StackType_t healthCheckTaskStack[HEALTH_CHECK_TASK_STACK_DEPTH];

/**
  * @brief Print the synchronized UTC time ahead of one check iteration.
  * @note Called only while TimeService_IsSynchronized() is true, so a
  *       conversion failure here indicates an internal RTC/library fault
  *       rather than a missing synchronization.
  */
static void healthCheckService_PrintRtc(void) {
  uint32_t unixTime;
  if (TimeService_GetUnixTime(&unixTime) != HEALTH_CHECK_STATUS_OK) {
    Common_Printf("RTC: unavailable\r\n");
    return;
  }
  time_t timestamp = (time_t)unixTime;
  struct tm utc;
  if (gmtime_r(&timestamp, &utc) == NULL) {
    Common_Printf("RTC: conversion failed\r\n");
    return;
  }
  Common_Printf(
    "RTC: %04d-%02d-%02dT%02d:%02d:%02dZ\r\n",
    utc.tm_year + 1900,
    utc.tm_mon + 1,
    utc.tm_mday,
    utc.tm_hour,
    utc.tm_min,
    utc.tm_sec
  );
}

/**
  * @brief Run one check against a configured resource and record the result.
  * @param index (uint8_t) Configured resource slot.
  * @param resource (const HealthCheckConfig_ResourceTypeDef*) Occupied,
  *        enabled resource to check.
  */
static void healthCheckService_CheckResource(
  uint8_t index,
  const HealthCheckConfig_ResourceTypeDef* resource
) {
  Common_Printf(
    "HTTPS check: https://%s%s\r\n", resource->host, resource->path
  );
  TlsTransport_ResultTypeDef result;
  TlsTransport_Head(
    resource->host,
    resource->port,
    resource->path,
    resource->trustAnchorId,
    &result
  );

  if (result.status == TLS_TRANSPORT_OK) {
    Common_Printf("TLS: %s, %s, certificate valid\r\n",
      result.tlsVersion,
      result.cipherSuite);
    Common_Printf("HTTP HEAD: %u, %lu ms\r\n",
      result.httpStatus,
      (unsigned long)result.elapsedMs);
  } else {
    Common_Printf("HTTPS failure: stage=%u, detail=%d, %lu ms\r\n",
      (unsigned int)result.status,
      result.detail,
      (unsigned long)result.elapsedMs);
  }

  uint8_t resourceHealthy = ((result.status == TLS_TRANSPORT_OK)
      && (result.httpStatus == HEALTH_CHECK_OK_STATUS))
    ? 1U
    : 0U;
  Common_Printf(
    "Resource health: %s\r\n", resourceHealthy != 0U ? "OK" : "FAILED"
  );
  if (resourceHealthy == 0U)
    BuzzerService_Alert();

  (void)HealthCheckLog_Append(index, &result);
}

static void healthCheckService_Task(void* argument) {
  (void)argument;

  if ((TlsPlatform_Init() != HEALTH_CHECK_STATUS_OK)
      || (TlsTrustStore_Init() != HEALTH_CHECK_STATUS_OK)
      || (HealthCheckConfig_Init() != HEALTH_CHECK_STATUS_OK)
      || (HealthCheckLog_Init() != HEALTH_CHECK_STATUS_OK)
      || (TlsServerCredentials_Init() != HEALTH_CHECK_STATUS_OK)
      || (ApiService_Init() != HEALTH_CHECK_STATUS_OK)) {
    System_ErrorHandler();
  }
  Common_Printf("HTTPS health checker: storage and TLS ready.\n");

  for (;;) {
    if ((NetworkService_IsReady() == 0U) || (TimeService_IsSynchronized() == 0U)) {
      vTaskDelay(pdMS_TO_TICKS(HEALTH_CHECK_WAIT_MS));
      continue;
    }

    healthCheckService_PrintRtc();

    HealthCheckConfig_ResourceTypeDef resources[
      HEALTH_CHECK_CONFIG_MAX_RESOURCES
    ];
    HealthCheckConfig_GetResources(resources);
    for (uint8_t index = 0U; index < HEALTH_CHECK_CONFIG_MAX_RESOURCES; ++index) {
      if ((resources[index].occupied == 0U)
          || (resources[index].enabled == 0U))
        continue;
      healthCheckService_CheckResource(index, &resources[index]);
    }

    vTaskDelay(pdMS_TO_TICKS(
      HealthCheckConfig_GetPeriodSeconds() * 1000U
    ));
  }
}

ErrorStatus HealthCheckService_Init(void) {
  TaskHandle_t task = xTaskCreateStatic(
    healthCheckService_Task,
    "health-check",
    HEALTH_CHECK_TASK_STACK_DEPTH,
    NULL,
    tskIDLE_PRIORITY + 1U,
    healthCheckTaskStack,
    &healthCheckTaskControlBlock
  );
  return task != NULL ? SUCCESS : ERROR;
}
