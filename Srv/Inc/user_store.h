/**
  ******************************************************************************
  * @file           : user_store.h
  * @brief          : Persistent management-user database.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 31.07.2026
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

#ifndef USER_STORE_H
#define USER_STORE_H

#include "main.h"

#include <stddef.h>
#include <stdint.h>

#define USER_STORE_MAX_USERS       8U
#define USER_STORE_USERNAME_SIZE   25U
#define USER_STORE_SALT_SIZE       16U
#define USER_STORE_VERIFIER_SIZE   32U

/** @brief Persistent management authorization roles. */
typedef enum {
  USER_ROLE_USER = 0,
  USER_ROLE_ADMINISTRATOR
} UserStore_RoleTypeDef;

/**
  * @brief One persistent management-user verifier record.
  * @param enabled (uint8_t) Nonzero when login is allowed.
  * @param role (uint8_t) UserStore_RoleTypeDef authorization role.
  * @param username (char[]) Null-terminated unique account name.
  * @param salt (uint8_t[]) Per-account PBKDF2 salt.
  * @param verifier (uint8_t[]) PBKDF2-HMAC-SHA-256 password verifier.
  * @param iterations (uint32_t) PBKDF2 iteration count.
  */
typedef struct {
  uint8_t enabled;
  uint8_t role;
  char username[USER_STORE_USERNAME_SIZE];
  uint8_t salt[USER_STORE_SALT_SIZE];
  uint8_t verifier[USER_STORE_VERIFIER_SIZE];
  uint32_t iterations;
} UserStore_RecordTypeDef;

/**
  * @brief Load the newest valid user snapshot or persist an empty store.
  * @retval (HealthCheck_StatusTypeDef) Storage initialization result.
  */
HealthCheck_StatusTypeDef UserStore_Init(void);
/**
  * @brief Find one enabled or disabled persistent account by username.
  * @param username (const char*) Non-null account name.
  * @param record (UserStore_RecordTypeDef*) Non-null record output.
  * @param index (uint8_t*) Optional persistent slot output; may be null.
  * @retval (HealthCheck_StatusTypeDef) OK when the account exists.
  */
HealthCheck_StatusTypeDef UserStore_Find(
  const char* username,
  UserStore_RecordTypeDef* record,
  uint8_t* index
);
/**
  * @brief Create or replace one persistent user record.
  * @param record (const UserStore_RecordTypeDef*) Valid record to persist.
  * @retval (HealthCheck_StatusTypeDef) Validation or storage result.
  */
HealthCheck_StatusTypeDef UserStore_Put(const UserStore_RecordTypeDef* record);
/**
  * @brief Delete one persistent account by username.
  * @param username (const char*) Non-null account name.
  * @retval (HealthCheck_StatusTypeDef) Lookup or storage result.
  */
HealthCheck_StatusTypeDef UserStore_Delete(const char* username);
/**
  * @brief Copy persistent user records into caller-owned storage.
  * @param records (UserStore_RecordTypeDef*) Non-null output array.
  * @param capacity (size_t) Available elements in records.
  * @retval (size_t) Number of copied records.
  */
size_t UserStore_List(UserStore_RecordTypeDef* records, size_t capacity);

#endif /* USER_STORE_H */
