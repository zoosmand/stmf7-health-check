/**
  ******************************************************************************
  * @file           : auth_service.h
  * @brief          : Master-password verification for the management API.
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

#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <stddef.h>
#include <stdint.h>

#include "user_store.h"

#define AUTH_SERVICE_TOKEN_TEXT_SIZE 65U

/**
  * @brief Result of master-password verification.
  */
typedef enum {
  AUTH_SERVICE_STATUS_OK = 0,
  AUTH_SERVICE_STATUS_INVALID_PASSWORD,
  AUTH_SERVICE_STATUS_INVALID_ARGUMENT,
  AUTH_SERVICE_STATUS_CRYPTO_ERROR,
  AUTH_SERVICE_STATUS_NOT_FOUND,
  AUTH_SERVICE_STATUS_FORBIDDEN,
  AUTH_SERVICE_STATUS_STORAGE_ERROR
} AuthService_StatusTypeDef;

/**
  * @brief Newly issued access and refresh tokens with their lifetimes.
  * @param accessToken (char[]) Null-terminated access-token text.
  * @param refreshToken (char[]) Null-terminated refresh-token text.
  * @param accessExpiresIn (uint32_t) Access-token lifetime in seconds.
  * @param refreshExpiresIn (uint32_t) Refresh-token lifetime in seconds.
  */
typedef struct {
  char accessToken[AUTH_SERVICE_TOKEN_TEXT_SIZE];
  char refreshToken[AUTH_SERVICE_TOKEN_TEXT_SIZE];
  uint32_t accessExpiresIn;
  uint32_t refreshExpiresIn;
} AuthService_TokenPairTypeDef;

/**
  * @brief Identity and authorization role resolved from an access token.
  * @param username (char[]) Null-terminated management username.
  * @param role (UserStore_RoleTypeDef) Effective management role.
  */
typedef struct {
  char username[USER_STORE_USERNAME_SIZE];
  UserStore_RoleTypeDef role;
} AuthService_PrincipalTypeDef;

/**
  * @brief Initialize session state and the persistent user store.
  * @retval (AuthService_StatusTypeDef) AUTH_SERVICE_STATUS_OK on success.
  */
AuthService_StatusTypeDef AuthService_Init(void);

/**
  * @brief Verify a candidate master password against the Flash-resident
  *        PBKDF2-HMAC-SHA-256 verifier.
  * @param password (const uint8_t*) Non-null password bytes.
  * @param passwordLength (size_t) Password length from 1 through 128 bytes.
  * @retval (AuthService_StatusTypeDef) AUTH_SERVICE_STATUS_OK only when the
  *         supplied password matches.
  * @note This function blocks while deriving the verifier and must run in task
  *       context. It never logs or retains the supplied password.
  */
AuthService_StatusTypeDef AuthService_VerifyMasterPassword(
  const uint8_t* password,
  size_t passwordLength
);

/**
  * @brief Authenticate one account and rotate its active token pair.
  * @param username (const char*) Non-null account name.
  * @param password (const uint8_t*) Non-null password bytes.
  * @param passwordLength (size_t) Password length from 1 through 128 bytes.
  * @param tokens (AuthService_TokenPairTypeDef*) Non-null issued-token output.
  * @retval (AuthService_StatusTypeDef) Authentication or storage result.
  */
AuthService_StatusTypeDef AuthService_Login(
  const char* username,
  const uint8_t* password,
  size_t passwordLength,
  AuthService_TokenPairTypeDef* tokens
);
/**
  * @brief Exchange a valid refresh token for a rotated token pair.
  * @param refreshToken (const char*) Non-null refresh-token text.
  * @param tokens (AuthService_TokenPairTypeDef*) Non-null issued-token output.
  * @retval (AuthService_StatusTypeDef) Authorization result.
  */
AuthService_StatusTypeDef AuthService_Refresh(
  const char* refreshToken,
  AuthService_TokenPairTypeDef* tokens
);
/**
  * @brief Resolve a valid access token to its authenticated principal.
  * @param accessToken (const char*) Non-null access-token text.
  * @param principal (AuthService_PrincipalTypeDef*) Non-null identity output.
  * @retval (AuthService_StatusTypeDef) Authorization result.
  */
AuthService_StatusTypeDef AuthService_Authorize(
  const char* accessToken,
  AuthService_PrincipalTypeDef* principal
);
/**
  * @brief Revoke the session identified by an access token.
  * @param accessToken (const char*) Non-null access-token text.
  * @retval (AuthService_StatusTypeDef) Revocation result.
  */
AuthService_StatusTypeDef AuthService_Revoke(const char* accessToken);
/**
  * @brief Create or update a persistent management account.
  * @param actor (const AuthService_PrincipalTypeDef*) Authenticated caller.
  * @param username (const char*) Target account name.
  * @param password (const uint8_t*) New password bytes.
  * @param passwordLength (size_t) Password length from 1 through 128 bytes.
  * @param role (UserStore_RoleTypeDef) New account role.
  * @param enabled (uint8_t) Nonzero to enable the account.
  * @param mustExist (uint8_t) Nonzero to reject creation of a new account.
  * @retval (AuthService_StatusTypeDef) Authorization or persistence result.
  */
AuthService_StatusTypeDef AuthService_PutUser(
  const AuthService_PrincipalTypeDef* actor,
  const char* username,
  const uint8_t* password,
  size_t passwordLength,
  UserStore_RoleTypeDef role,
  uint8_t enabled,
  uint8_t mustExist
);
/**
  * @brief Delete a persistent management account.
  * @param actor (const AuthService_PrincipalTypeDef*) Authenticated caller.
  * @param username (const char*) Target account name.
  * @retval (AuthService_StatusTypeDef) Authorization or persistence result.
  */
AuthService_StatusTypeDef AuthService_DeleteUser(
  const AuthService_PrincipalTypeDef* actor,
  const char* username
);
/**
  * @brief Copy persistent management accounts for an administrator.
  * @param actor (const AuthService_PrincipalTypeDef*) Authenticated caller.
  * @param records (UserStore_RecordTypeDef*) Non-null output array.
  * @param capacity (size_t) Available elements in records.
  * @retval (size_t) Number of copied records; zero when unauthorized.
  */
size_t AuthService_ListUsers(
  const AuthService_PrincipalTypeDef* actor,
  UserStore_RecordTypeDef* records,
  size_t capacity
);

#endif /* AUTH_SERVICE_H */
