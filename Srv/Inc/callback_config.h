#ifndef CALLBACK_CONFIG_H
#define CALLBACK_CONFIG_H

#include "health_check_types.h"

#include <stdint.h>

#define CALLBACK_CONFIG_HOST_SIZE 64U
#define CALLBACK_CONFIG_PATH_SIZE 80U

typedef enum {
  CALLBACK_METHOD_GET = 0,
  CALLBACK_METHOD_POST = 1
} CallbackConfig_MethodTypeDef;

/**
  * @brief Persisted outbound callback destination and delivery policy.
  * @param enabled (uint8_t) Nonzero when failed checks are delivered.
  * @param method (uint8_t) CallbackConfig_MethodTypeDef wire method.
  * @param trustAnchorId (uint8_t) Persistent CA slot used for validation.
  * @param port (uint16_t) Nonzero TCP destination port.
  * @param host (char[CALLBACK_CONFIG_HOST_SIZE]) Null-terminated DNS host.
  * @param path (char[CALLBACK_CONFIG_PATH_SIZE]) Origin-form request path.
  */
typedef struct {
  uint8_t enabled;
  uint8_t method;
  uint8_t trustAnchorId;
  uint8_t reserved;
  uint16_t port;
  char host[CALLBACK_CONFIG_HOST_SIZE];
  char path[CALLBACK_CONFIG_PATH_SIZE];
} CallbackConfig_TypeDef;

/** @brief Load the newest valid A/B snapshot or persist defaults. */
HealthCheck_StatusTypeDef CallbackConfig_Init(void);

/**
  * @brief Copy the current callback configuration.
  * @param config (CallbackConfig_TypeDef*) Non-null output storage.
  */
void CallbackConfig_Get(CallbackConfig_TypeDef* config);

/**
  * @brief Validate and transactionally persist a complete configuration.
  * @param config (const CallbackConfig_TypeDef*) Non-null candidate.
  * @retval (HealthCheck_StatusTypeDef) OK after verified persistent storage.
  */
HealthCheck_StatusTypeDef CallbackConfig_Set(
  const CallbackConfig_TypeDef* config
);

/**
  * @brief Report whether an enabled callback references a trust anchor.
  * @param trustAnchorId (uint8_t) Persistent anchor ID to query.
  * @retval (uint8_t) Nonzero when the anchor is in active use.
  */
uint8_t CallbackConfig_IsTrustAnchorInUse(uint8_t trustAnchorId);

#endif /* CALLBACK_CONFIG_H */
