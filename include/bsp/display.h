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

#ifndef __BSP_DISPLAY_H__
#define __BSP_DISPLAY_H__

#include "pmsis.h"
#include "bsp/buffer.h"


typedef enum
{
  DISPLAY_IOCTL_CUSTOM = 0
} display_ioctl_cmd_e;


/** \brief Open an image sensor device.
 *
 * This function must be called before the Camera device can be used. It configure the device
 * and return a handle that can be used to refer to the opened device when calling other functions.
 * This operation is asynchronous and its termination can be managed through an event.
 * Can only be called from fabric-controller side.
 *
 * \param dev_name  The device name. This name should correspond to the one used to configure the devices managed by the runtime.
 * \param conf      A pointer to the Camera device configuration. Can be NULL to use the default configuration.
 * \param event     The event used for managing termination.
 * \return          NULL if the device is not found, or a handle identifying the device which can be used with other Camera functions.
 */
int display_open(struct pi_device *device);

void display_write(struct pi_device *device, pi_buffer_t *buffer, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

static inline int32_t display_ioctl(struct pi_device *device, uint32_t cmd, void *arg);

static inline void display_write_async(struct pi_device *device, pi_buffer_t *buffer, uint16_t x, uint16_t y,uint16_t w, uint16_t h, pi_task_t *task);


//!@}

/**
 * @}
 */


/// @cond IMPLEM

typedef struct {
  int (*open)(struct pi_device *device);
  void (*write_async)(struct pi_device *device, pi_buffer_t *buffer, uint16_t x, uint16_t y,uint16_t w, uint16_t h, pi_task_t *task);
  int32_t (*ioctl)(struct pi_device *device, uint32_t cmd, void *arg);
} display_api_t;

struct display_conf {
  display_api_t *api;
};

static inline void display_write_async(struct pi_device *device, pi_buffer_t *buffer, uint16_t x, uint16_t y,uint16_t w, uint16_t h, pi_task_t *task)
{
  display_api_t *api = (display_api_t *)device->api;
  api->write_async(device, buffer, x, y, w, h, task);
}

void __display_conf_init(struct display_conf *conf);

static inline int32_t display_ioctl(struct pi_device *device, uint32_t cmd, void *arg)
{
  display_api_t *api = (display_api_t *)device->api;
  return api->ioctl(device, cmd, arg);
}


/// @endcond




#endif
