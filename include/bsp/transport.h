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

#ifndef __BSP_TRANSPORT_H__
#define __BSP_TRANSPORT_H__

#include "pmsis.h"


#define TRANSPORT_USER_FIRST_CHANNEL 1024

struct transport_header
{
  uint32_t channel;
  uint32_t packet_size;
};

int transport_open(struct pi_device *device);

int transport_connect(struct pi_device *device, void (*rcv_callback(void *arg, struct transport_header)), void *arg);

int transport_send_header_async(struct pi_device *device, struct transport_header *header, int channel, size_t size, pi_task_t *task);

int transport_send_header(struct pi_device *device, struct transport_header *header, int channel, size_t size);

static inline int transport_send_async(struct pi_device *device, void *buffer, size_t size, pi_task_t *task);

int transport_send(struct pi_device *device, void *buffer, size_t size);

static inline int transport_receive_async(struct pi_device *device, void *buffer, size_t size, pi_task_t *task);

int transport_receive(struct pi_device *device, void *buffer, size_t size);

static inline void transport_close(struct pi_device *device);




//!@}

/**
 * @}
 */


/// @cond IMPLEM

typedef struct {
  int (*open)(struct pi_device *device);
  int (*connect)(struct pi_device *device, int channel, void (*rcv_callback(void *arg, struct transport_header)), void *arg);
  int (*send_async)(struct pi_device *device, void *buffer, size_t size, pi_task_t *task);
  int (*receive_async)(struct pi_device *device, void *buffer, size_t size, pi_task_t *task);
  void (*close)(struct pi_device *device);
} transport_api_t;

struct transport_conf {
  transport_api_t *api;
};

static inline int transport_send_async(struct pi_device *device, void *buffer, size_t size, pi_task_t *task)
{
  transport_api_t *api = (transport_api_t *)device->api;
  return api->send_async(device, buffer, size, task);
}

static inline int transport_receive_async(struct pi_device *device, void *buffer, size_t size, pi_task_t *task)
{
  transport_api_t *api = (transport_api_t *)device->api;
  return api->receive_async(device, buffer, size, task);
}

static inline void transport_close(struct pi_device *device)
{
  transport_api_t *api = (transport_api_t *)device->api;
  api->close(device);
}

/// @endcond




#endif
