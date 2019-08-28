/*
 * Copyright (C) 2019 GreenWaves Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BSP_FLASH_H__
#define __BSP_FLASH_H__

#include "pmsis.h"

typedef enum {
  FLASH_TYPE_SPI,     /*!< SPI flash. */
  FLASH_TYPE_HYPERFLASH,    /*!< Hyperflash. */
  FLASH_TYPE_MRAM     /*!< MRAM. */
} flash_type_e;

typedef enum {
  FLASH_IOCTL_INFO
} flash_ioctl_e;

struct flash_info {
  uint32_t sector_size;
  uint32_t flash_start;
};

int flash_open(struct pi_device *device);

static inline void flash_close(struct pi_device *device);

static inline void flash_ioctl(struct pi_device *device, uint32_t cmd, void *arg);

static inline void flash_reg_set_async(struct pi_device *device, uint32_t flash_addr, uint8_t *value, pi_task_t *task);

static inline void flash_reg_set(struct pi_device *device, uint32_t flash_addr, uint8_t *value);

static inline void flash_reg_get_async(struct pi_device *device, uint32_t flash_addr, uint8_t *value, pi_task_t *task);

static inline void flash_reg_get(struct pi_device *device, uint32_t flash_addr, uint8_t *value);

static inline void flash_read_async(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size, pi_task_t *task);

static inline void flash_read(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size);

static inline void flash_program_async(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size, pi_task_t *task);

static inline void flash_program(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size);

static inline void flash_erase_chip_async(struct pi_device *device, pi_task_t *task);

static inline void flash_erase_chip(struct pi_device *device);

static inline void flash_erase_sector_async(struct pi_device *device, uint32_t flash_addr, pi_task_t *task);

static inline void flash_erase_sector(struct pi_device *device, uint32_t flash_addr);

static inline void flash_erase_async(struct pi_device *device, uint32_t flash_addr, int size, pi_task_t *task);

static inline void flash_erase(struct pi_device *device, uint32_t flash_addr, int size);

/// @cond IMPLEM

typedef struct {
  int (*open)(struct pi_device *device);
  void (*close)(struct pi_device *device);
  void (*ioctl)(struct pi_device *device, uint32_t cmd, void *arg);
  void (*read_async)(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size, pi_task_t *task);
  void (*program_async)(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size, pi_task_t *task);
  void (*erase_chip_async)(struct pi_device *device, pi_task_t *task);
  void (*erase_sector_async)(struct pi_device *device, uint32_t flash_addr, pi_task_t *task);
  void (*erase_async)(struct pi_device *device, uint32_t flash_addr, int size, pi_task_t *task);
  void (*reg_set_async)(struct pi_device *device, uint32_t flash_addr, uint8_t *value, pi_task_t *task);
  void (*reg_get_async)(struct pi_device *device, uint32_t flash_addr, uint8_t *value, pi_task_t *task);
} flash_api_t;

struct flash_conf {
  flash_api_t *api;
  flash_type_e type;
};


static inline void flash_close(struct pi_device *device)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->close(device);
}

static inline void flash_ioctl(struct pi_device *device, uint32_t cmd, void *arg)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->ioctl(device, cmd, arg);
}

static inline void flash_reg_set_async(struct pi_device *device, uint32_t flash_addr, uint8_t *value, pi_task_t *task)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->reg_set_async(device, flash_addr, value, task);
}

static inline void flash_reg_set(struct pi_device *device, uint32_t flash_addr, uint8_t *value)
{
  pi_task_t task;
  flash_reg_set_async(device, flash_addr, value, pi_task_block(&task));
  pi_task_wait_on(&task);
}

static inline void flash_reg_get_async(struct pi_device *device, uint32_t flash_addr, uint8_t *value, pi_task_t *task)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->reg_get_async(device, flash_addr, value, task);
}

static inline void flash_reg_get(struct pi_device *device, uint32_t flash_addr, uint8_t *value)
{
  pi_task_t task;
  flash_reg_get_async(device, flash_addr, value, pi_task_block(&task));
  pi_task_wait_on(&task);
}

static inline void flash_read_async(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size, pi_task_t *task)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->read_async(device, flash_addr, data, size, task);
}

static inline void flash_read(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size)
{
  pi_task_t task;
  flash_read_async(device, flash_addr, data, size, pi_task_block(&task));
  pi_task_wait_on(&task);
}

static inline void flash_program_async(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size, pi_task_t *task)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->program_async(device, flash_addr, data, size, task);
}

static inline void flash_program(struct pi_device *device, uint32_t flash_addr, void *data, uint32_t size)
{
  pi_task_t task;
  flash_program_async(device, flash_addr, data, size, pi_task_block(&task));
  pi_task_wait_on(&task);
}

static inline void flash_erase_chip_async(struct pi_device *device, pi_task_t *task)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->erase_chip_async(device, task);
}

static inline void flash_erase_chip(struct pi_device *device)
{
  pi_task_t task;
  flash_erase_chip_async(device, pi_task_block(&task));
  pi_task_wait_on(&task);
}

static inline void flash_erase_sector_async(struct pi_device *device, uint32_t flash_addr, pi_task_t *task)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->erase_sector_async(device, flash_addr, task);
}

static inline void flash_erase_sector(struct pi_device *device, uint32_t flash_addr)
{
  pi_task_t task;
  flash_erase_sector_async(device, flash_addr, pi_task_block(&task));
  pi_task_wait_on(&task);
}

static inline void flash_erase_async(struct pi_device *device, uint32_t flash_addr, int size, pi_task_t *task)
{
  flash_api_t *api = (flash_api_t *)device->api;
  api->erase_async(device, flash_addr, size, task);
}

static inline void flash_erase(struct pi_device *device, uint32_t flash_addr, int size)
{
  pi_task_t task;
  flash_erase_async(device, flash_addr, size, pi_task_block(&task));
  pi_task_wait_on(&task);
}

void __flash_conf_init(struct flash_conf *conf);

/// @endcond




#endif
