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

#ifndef __BSP_RAM_H__
#define __BSP_RAM_H__

#include "pmsis.h"

int ram_open(struct pi_device *device);

static inline void ram_close(struct pi_device *device);

static inline void ram_read_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task);

static inline void ram_read(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size);


static inline void ram_write_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task);

static inline void ram_write(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size);

static inline int ram_alloc(struct pi_device *device, uint32_t size, uint32_t *addr);

static inline int ram_free(struct pi_device *device, uint32_t size, uint32_t addr);

/// @cond IMPLEM

typedef struct {
  int (*open)(struct pi_device *device);
  void (*close)(struct pi_device *device);
  void (*read_async)(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task);
  void (*write_async)(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task);
  int (*alloc)(struct pi_device *device, uint32_t size, uint32_t *addr);
  int (*free)(struct pi_device *device, uint32_t size, uint32_t addr);
} ram_api_t;

struct ram_conf {
  ram_api_t *api;
};


static inline void ram_close(struct pi_device *device)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->close(device);
}

static inline void ram_read_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->read_async(device, ram_addr, data, size, task);
}

static inline void ram_write_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->write_async(device, ram_addr, data, size, task);
}

static inline void ram_read(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size)
{
  pi_task_t task;
  ram_read_async(device, ram_addr, data, size, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline void ram_write(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size)
{
  pi_task_t task;
  ram_write_async(device, ram_addr, data, size, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline int ram_alloc(struct pi_device *device, uint32_t size, uint32_t *addr)
{
  ram_api_t *api = (ram_api_t *)device->api;
  return api->alloc(device, size, addr);
}

static inline int ram_free(struct pi_device *device, uint32_t size, uint32_t addr)
{
  ram_api_t *api = (ram_api_t *)device->api;
  return api->free(device, size, addr);
}

void __ram_conf_init(struct ram_conf *conf);

/// @endcond




#endif
