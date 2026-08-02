/**
  ******************************************************************************
  * @file           : sys_arch.h
  * @brief          : Static FreeRTOS types for the lwIP system API.
  * @project        : STM32F767 Health Check
  * @platform       : STMicroelectronics STM32F767ZIT6
  * @created        : 29.07.2025
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2001-2003 Swedish Institute of Computer Science
  * Copyright (c) 2017-2026 Dmitry Slobodchikov
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "lwip/arch.h"

#define SYS_ARCH_MBOX_CAPACITY 16U

/**
  * @brief Statically allocated lwIP binary semaphore.
  * @param handle (SemaphoreHandle_t) FreeRTOS semaphore handle.
  * @param storage (StaticSemaphore_t) Semaphore control-block storage.
  */
typedef struct {
  SemaphoreHandle_t handle;
  StaticSemaphore_t storage;
} sys_sem_t;

/**
  * @brief Statically allocated lwIP recursive mutex.
  * @param handle (SemaphoreHandle_t) FreeRTOS mutex handle.
  * @param storage (StaticSemaphore_t) Mutex control-block storage.
  */
typedef struct {
  SemaphoreHandle_t handle;
  StaticSemaphore_t storage;
} sys_mutex_t;

/**
  * @brief Statically allocated lwIP message mailbox.
  * @param handle (QueueHandle_t) FreeRTOS queue handle.
  * @param queue (StaticQueue_t) Queue control-block storage.
  * @param messages (void*[]) Fixed queue item storage.
  */
typedef struct {
  QueueHandle_t handle;
  StaticQueue_t queue;
  void* messages[SYS_ARCH_MBOX_CAPACITY];
} sys_mbox_t;

typedef TaskHandle_t sys_thread_t;

#define SYS_SEM_NULL  ((sys_sem_t){0})
#define SYS_MBOX_NULL ((sys_mbox_t){0})

#define sys_sem_valid(semaphore) \
  (((semaphore) != NULL) && ((semaphore)->handle != NULL))
#define sys_sem_set_invalid(semaphore) ((semaphore)->handle = NULL)

#define sys_mutex_valid(mutex) \
  (((mutex) != NULL) && ((mutex)->handle != NULL))
#define sys_mutex_set_invalid(mutex) ((mutex)->handle = NULL)

#define sys_mbox_valid(mailbox) \
  (((mailbox) != NULL) && ((mailbox)->handle != NULL))
#define sys_mbox_set_invalid(mailbox) ((mailbox)->handle = NULL)

void sys_arch_msleep(u32_t delayMs);
#define sys_msleep(delayMs) sys_arch_msleep(delayMs)

#endif /* LWIP_ARCH_SYS_ARCH_H */
