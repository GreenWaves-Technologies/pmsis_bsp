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
#include "rtos/os_frontend_api/pmsis_task.h"
#if defined(PMSIS_DRIVERS)
#include "pmsis_cluster/drivers/delegate/hyperbus/hyperbus_cl_internal.h"
#endif

typedef struct cl_ram_req_s cl_ram_req_t;

typedef struct cl_ram_alloc_req_s cl_ram_alloc_req_t;

typedef struct cl_ram_free_req_s cl_ram_free_req_t;

int ram_open(struct pi_device *device);

static inline void ram_close(struct pi_device *device);

static inline void ram_read_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task);

static inline void ram_read_2d_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, pi_task_t *task);

static inline void ram_read(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size);

static inline void ram_read_2d(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length);

static inline void ram_write_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task);

static inline void ram_write_2d_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, pi_task_t *task);

static inline void ram_write(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size);

static inline void ram_write_2d(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length);

static inline void ram_copy_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, int ext2loc, pi_task_t *task);

static inline void ram_copy_2d_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, pi_task_t *task);

static inline void ram_copy(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, int ext2loc);

static inline void ram_copy_2d(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, int ext2loc);

static inline int ram_alloc(struct pi_device *device, uint32_t *addr, uint32_t size);

static inline int ram_free(struct pi_device *device, uint32_t addr, uint32_t size);

static inline void cl_ram_read(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, cl_ram_req_t *req);

static inline void cl_ram_read_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, cl_ram_req_t *req);

static inline void cl_ram_read_wait(cl_ram_req_t *req);

static inline void cl_ram_write(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, cl_ram_req_t *req);

static inline void cl_ram_write_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, cl_ram_req_t *req);

static inline void cl_ram_write_wait(cl_ram_req_t *req);

void cl_ram_copy(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, int ext2loc, cl_ram_req_t *req);

void cl_ram_copy_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, cl_ram_req_t *req);

static inline void cl_ram_copy_wait(cl_ram_req_t *req);

void cl_ram_alloc(struct pi_device *device, uint32_t size, cl_ram_alloc_req_t *req);

void cl_ram_free(struct pi_device *device, uint32_t chunk, uint32_t size, cl_ram_free_req_t *req);

static inline int cl_ram_alloc_wait(cl_ram_alloc_req_t *req, uint32_t *chunk);

static inline void cl_ram_free_wait(cl_ram_free_req_t *req);

/// @cond IMPLEM

struct cl_ram_req_s {
  struct pi_device *device;
  void *addr;
  uint32_t hyper_addr;
  uint32_t size;
  int32_t stride;
  uint32_t length;
  pi_task_t event;
#if defined(PMSIS_DRIVERS)
  pi_task_t done;
#else
  struct pi_cl_hyper_req_s *next;
  int done;
#endif
  unsigned char cid;
  unsigned char ext2loc;
  unsigned char is_2d;
};

struct cl_ram_alloc_req_s {
  struct pi_device *device;
  uint32_t result;
  uint32_t  size;
  pi_task_t event;
#if defined(PMSIS_DRIVERS)
  pi_task_t done;
#else
  int done;
#endif
  char cid;
  char error;
};

struct cl_ram_free_req_s {
  struct pi_device *device;
  uint32_t result;
  uint32_t size;
  uint32_t chunk;
  pi_task_t event;
#if defined(PMSIS_DRIVERS)
  pi_task_t done;
#else
  int done;
#endif
  char cid;
};


typedef struct {
  int (*open)(struct pi_device *device);
  void (*close)(struct pi_device *device);
  void (*copy_async)(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, int ext2loc, pi_task_t *task);
  void (*copy_2d_async)(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, pi_task_t *task);
  int (*alloc)(struct pi_device *device, uint32_t *addr, uint32_t size);
  int (*free)(struct pi_device *device, uint32_t addr, uint32_t size);
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
  api->copy_async(device, ram_addr, data, size, 1, task);
}

static inline void ram_read_2d_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, pi_task_t *task)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->copy_2d_async(device, ram_addr, data, size, stride, length, 1, task);
}

static inline void ram_write_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, pi_task_t *task)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->copy_async(device, ram_addr, data, size, 0, task);
}

static inline void ram_write_2d_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, pi_task_t *task)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->copy_2d_async(device, ram_addr, data, size, stride, length, 0, task);
}

static inline void ram_copy_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, int ext2loc, pi_task_t *task)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->copy_async(device, ram_addr, data, size, ext2loc, task);
}

static inline void ram_copy_2d_async(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, pi_task_t *task)
{
  ram_api_t *api = (ram_api_t *)device->api;
  api->copy_2d_async(device, ram_addr, data, size, stride, length, ext2loc, task);
}

static inline void ram_read(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size)
{
  pi_task_t task;
  ram_read_async(device, ram_addr, data, size, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline void ram_read_2d(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length)
{
  pi_task_t task;
  ram_read_2d_async(device, ram_addr, data, size, stride, length, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline void ram_copy(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, int ext2loc)
{
  pi_task_t task;
  ram_copy_async(device, ram_addr, data, size, ext2loc, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline void ram_copy_2d(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length, int ext2loc)
{
  pi_task_t task;
  ram_copy_2d_async(device, ram_addr, data, size, stride, length, ext2loc, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline void ram_write(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size)
{
  pi_task_t task;
  ram_write_async(device, ram_addr, data, size, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline void ram_write_2d(struct pi_device *device, uint32_t ram_addr, void *data, uint32_t size, uint32_t stride, uint32_t length)
{
  pi_task_t task;
  ram_write_2d_async(device, ram_addr, data, size, stride, length, pi_task(&task));
  pi_task_wait_on(&task);
}

static inline int ram_alloc(struct pi_device *device, uint32_t *addr, uint32_t size)
{
  ram_api_t *api = (ram_api_t *)device->api;
  return api->alloc(device, addr, size);
}

static inline int ram_free(struct pi_device *device, uint32_t addr, uint32_t size)
{
  ram_api_t *api = (ram_api_t *)device->api;
  return api->free(device, addr, size);
}

void __ram_conf_init(struct ram_conf *conf);

static inline void cl_ram_read(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, cl_ram_req_t *req)
{
  cl_ram_copy(device, hyper_addr, addr, size, 1, req);
}

static inline void cl_ram_read_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, cl_ram_req_t *req)
{
  cl_ram_copy_2d(device, hyper_addr, addr, size, stride, length, 1, req);
}

static inline void cl_ram_read_wait(cl_ram_req_t *req)
{
  cl_ram_copy_wait(req);
}

static inline void cl_ram_write(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, cl_ram_req_t *req)
{
  cl_ram_copy(device, hyper_addr, addr, size, 0, req);
}

static inline void cl_ram_write_2d(struct pi_device *device,
  uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, cl_ram_req_t *req)
{
  cl_ram_copy_2d(device, hyper_addr, addr, size, stride, length, 0, req);
}

static inline void cl_ram_write_wait(cl_ram_req_t *req)
{
  cl_ram_copy_wait(req);
}

static inline void cl_ram_copy_wait(cl_ram_req_t *req)
{
#if defined(PMSIS_DRIVERS)
    pi_task_wait_on_no_mutex(&(req->done));
    hal_compiler_barrier();
#else
  while((*(volatile char *)&req->done) == 0)
  {
    eu_evt_maskWaitAndClr(1<<RT_CLUSTER_CALL_EVT);
  }
#endif
}

static inline int cl_ram_alloc_wait(cl_ram_alloc_req_t *req, uint32_t *chunk)
{
#if defined(PMSIS_DRIVERS)
    pi_task_wait_on_no_mutex(&(req->done));
    hal_compiler_barrier();
#else
  while((*(volatile char *)&req->done) == 0)
  {
    eu_evt_maskWaitAndClr(1<<RT_CLUSTER_CALL_EVT);
  }
#endif

  *chunk = req->result;

  return req->error;
}

static inline void cl_ram_free_wait(cl_ram_free_req_t *req)
{
#if defined(PMSIS_DRIVERS)
    pi_task_wait_on_no_mutex(&(req->done));
    hal_compiler_barrier();
#else
  while((*(volatile char *)&req->done) == 0)
  {
    eu_evt_maskWaitAndClr(1<<RT_CLUSTER_CALL_EVT);
  }
#endif
}

/// @endcond




#endif
