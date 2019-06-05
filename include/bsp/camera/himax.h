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

#ifndef __BSP_CAMERA_HIMAX_H__
#define __BSP_CAMERA_HIMAX_H__

#include "bsp/camera.h"

struct himax_conf
{
  struct camera_conf camera;
  int cpi_itf;
  int i2c_itf;
  char skip_pads_config;
  camera_format_e format;
};

/** \brief Initialize a camera configuration with default values.
 *
 * The structure containing the configuration must be kept alive until the camera device is opened.
 * Can only be called from fabric-controller side.
 *
 * \param conf A pointer to the camera configuration.
 */
void himax_conf_init(struct himax_conf *conf);




#endif
