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

/*
 * Authors: Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#include "pmsis/task.h"
#include "bsp/camera.h"


int camera_open(struct pi_device *device)
{
  struct camera_conf *conf = (struct camera_conf *)device->config;
  camera_api_t *api = (camera_api_t *)conf->api;
  device->api = (struct pi_device_api *)api;
  return api->open(device);
}



void camera_capture(struct pi_device *device, void *buffer, uint32_t size)
{
  pi_task_t task;
  camera_capture_async(device, buffer, size, pi_task_block(&task));
  pi_task_wait_on(&task);
}



void __camera_conf_init(struct camera_conf *conf)
{
}
