/**
  ******************************************************************************
  * @file           : health_check_log.h
  * @brief          : Wear-aware persistent ring of recent health-check results.
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

#ifndef HEALTH_CHECK_LOG_H
#define HEALTH_CHECK_LOG_H

#include "main.h"
#include "tls_transport.h"

#include <stddef.h>
#include <stdint.h>

#define HEALTH_CHECK_LOG_MAX_RESULTS 50U

/**
  * @brief One retained health-check result.
  * @param sequence (uint32_t) Monotonically increasing record number; higher
  *        is more recent.
  * @param timestampUnix (uint32_t) RTC UTC time at the moment of the check.
  * @param elapsedMs (uint32_t) Check duration in milliseconds.
  * @param detail (int32_t) Layer-specific diagnostic code; see
  *        TlsTransport_ResultTypeDef.detail.
  * @param certificateExpiresAt (uint32_t) Remote leaf-certificate expiry as
  *        Unix UTC, or zero when the peer certificate was unavailable.
  * @param certificateDaysLeft (int32_t) Whole days remaining at check time.
  * @note The transport currently captures expiry only after a successful TLS
  *       handshake. Expired certificates therefore report unavailable expiry
  *       metadata rather than a negative certificateDaysLeft value.
  * @param httpStatus (uint16_t) HTTP status received, or 0 if none.
  * @param resourceIndex (uint8_t) Configured resource slot this check ran
  *        against.
  * @param status (uint8_t) TlsTransport_StatusTypeDef value for this check.
  * @param certificateAnchorId (uint8_t) Configured trust-anchor ID.
  * @param certificateExpiring (uint8_t) Nonzero with less than ten days left.
  */
typedef struct {
  uint32_t sequence;
  uint32_t timestampUnix;
  uint32_t elapsedMs;
  int32_t detail;
  uint32_t certificateExpiresAt;
  int32_t certificateDaysLeft;
  uint16_t httpStatus;
  uint8_t resourceIndex;
  uint8_t status;
  uint8_t certificateAnchorId;
  uint8_t certificateExpiring;
} HealthCheckLog_EntryTypeDef;

HealthCheck_StatusTypeDef HealthCheckLog_Init(void);

/**
  * @brief Append one check result to the persistent ring.
  * @param resourceIndex (uint8_t) Configured resource slot this check ran
  *        against.
  * @param trustAnchorId (uint8_t) Configured trust anchor used by the check.
  * @param timestampUnix (uint32_t) RTC UTC time captured for this check and
  *        its certificate-warning decision.
  * @param result (const TlsTransport_ResultTypeDef*) Non-null check result.
  * @param entry (HealthCheckLog_EntryTypeDef*) Non-null output containing the
  *        exact record written and verified.
  * @retval (HealthCheck_StatusTypeDef) HEALTH_CHECK_STATUS_OK when the record was written and
  *         verified.
  */
HealthCheck_StatusTypeDef HealthCheckLog_Append(
  uint8_t resourceIndex,
  uint8_t trustAnchorId,
  uint32_t timestampUnix,
  const TlsTransport_ResultTypeDef* result,
  HealthCheckLog_EntryTypeDef* entry
);

/**
  * @brief Copy out the most recent records, newest first.
  * @param entries (HealthCheckLog_EntryTypeDef*) Non-null output array.
  * @param capacity (size_t) Available elements in entries, at most
  *        HEALTH_CHECK_LOG_MAX_RESULTS are ever meaningfully returned.
  * @retval (size_t) Number of records copied.
  */
size_t HealthCheckLog_GetRecent(
  HealthCheckLog_EntryTypeDef* entries,
  size_t capacity
);

#endif /* HEALTH_CHECK_LOG_H */
