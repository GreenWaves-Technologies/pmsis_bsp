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

#ifndef __BSP_CAMERA_H__
#define __BSP_CAMERA_H__

#include "pmsis.h"
#include "drivers/cpi.h"
#include "drivers/i2c.h"

typedef enum {
  CAMERA_CMD_ON,
  CAMERA_CMD_OFF,
  CAMERA_CMD_START,
  CAMERA_CMD_STOP
} camera_cmd_e;


typedef enum {
  CAMERA_VGA,
  CAMERA_QVGA,
  CAMERA_QQVGA
} camera_format_e;

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
int camera_open(struct pi_device *device);


/** \brief Control the Camera device.
 *
 * This function is used to control and configure the Camera device. For each command, the arguments necessary are listed below:
 *       CMD         |     Type of argument
 *    --|--
 *    CMD_RESOL      |     camera_resol_e
 *    CMD_FORMAT     |     ov7670_format_e OR himax_format_e
 *    CMD_FPS        |     camera_fps_e
 *    CMD_SLICE      |     img_slice_t
 *    CMD_FILTER     |     img_filter_t
 *    CMD_SHIFT      |     unsigned char
 *    CMD_FRAMEDROP  |     unsigned int
 *    CMD_START      |     NULL
 *    CMD_STOP       |     NULL
 *    CMD_PAUSE      |     NULL
 * Can only be called from fabric-controller side.
 *
 * \param handle    The handle of the device camera which returned when the device was opened.
 * \param cmd       The command for controlling or configuring the camera. Check the description of camera_cmd_e for further information.
 * \param *arg      A pointer to the arguments of the command.
 */
static inline void camera_control(struct pi_device *device, camera_cmd_e cmd, void *arg);

/** \brief Capture a sequence of samples.
 *
 * Queue a buffer that will receive Camera samples.
 * It is possible to call this function asynchronously and several times in order to queue several buffers.
 * At a minimum 2 buffers should be queued to ensure that no data sampled is lost. This is also the most efficient
 * way to retrieve data from the Camera device. You should always make sure that at least 2 buffers are always queued,
 * by queuing a new one as soon as the current one is full.
 * Can only be called from fabric-controller side.
 *
 * \param handle    The handle of the device which was returned when the device was opened.
 * \param buffer    The memory buffer where the captured samples will be transfered.
 * \param size      The size in bytes of the memory buffer.
 * \param event     The event used for managing termination. The event will be notified as soon as the specified amount of bytes have been captured.
 */
void camera_capture(struct pi_device *device, void *buffer, uint32_t size);

static inline void camera_capture_async(struct pi_device *device, void *buffer, uint32_t size, pi_task_t *task);

/** \brief Close an opened Camera device.
 *
 * This function can be called to close an opened Camera device once it is not needed anymore in order to free
 * all allocated resources. Once this function is called, the device is not accessible anymore and must be opened
 * again before being used.
 * This operation is asynchronous and its termination can be managed through an event.
 * Can only be called from fabric-controller side.
 *
 * \param handle    The handle of the device which was returned when it was opened.
 * \param event     The event used for managing termination.
 */
static inline void camera_close(struct pi_device *device);

static inline int camera_reg_set(struct pi_device *device, uint32_t addr, uint8_t *value);

static inline int camera_reg_get(struct pi_device *device, uint32_t addr, uint8_t *value);





//!@}

/**
 * @}
 */


/// @cond IMPLEM

typedef struct {
  int (*open)(struct pi_device *device);
  void (*close)(struct pi_device *device);
  void (*control)(struct pi_device *device, camera_cmd_e cmd, void *arg);
  void (*capture_async)(struct pi_device *device, void *buffer, uint32_t bufferlen, pi_task_t *task);
  int (*reg_get)(struct pi_device *device, uint32_t addr, uint8_t *value);
  int (*reg_set)(struct pi_device *device, uint32_t addr, uint8_t *value);
} camera_api_t;

struct camera_conf {
  int itf;
  camera_api_t *api;
};

static inline void camera_control(struct pi_device *device, camera_cmd_e cmd, void *arg)
{
  camera_api_t *api = (camera_api_t *)device->api;
  api->control(device, cmd, arg);
}

static inline void camera_capture_async(struct pi_device *device, void *buffer, uint32_t bufferlen, pi_task_t *task)
{
  camera_api_t *api = (camera_api_t *)device->api;
  api->capture_async(device, buffer, bufferlen, task);
}

static inline void camera_close(struct pi_device *device)
{
  camera_api_t *api = (camera_api_t *)device->api;
  api->close(device);
}

static inline int camera_reg_set(struct pi_device *device, uint32_t addr, uint8_t *value)
{
  camera_api_t *api = (camera_api_t *)device->api;
  return api->reg_set(device, addr, value);
}

static inline int camera_reg_get(struct pi_device *device, uint32_t addr, uint8_t *value)
{
  camera_api_t *api = (camera_api_t *)device->api;
  return api->reg_get(device, addr, value);
}

void __camera_conf_init(struct camera_conf *conf);

/// @endcond




#endif
