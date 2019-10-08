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

#include "pmsis.h"

#ifndef __BSP__DISPLAY__ILI9341_H__
#define __BSP__DISPLAY__ILI9341_H__

#include "bsp/display.h"

struct pi_ili9341_conf
{
  struct pi_display_conf display;
  int spi_itf;
  int spi_cs;
  int gpio;
  char skip_pads_config;
};



typedef enum
{
  PI_ILI_ORIENTATION_0 = 0,
  PI_ILI_ORIENTATION_90 = 1,
  PI_ILI_ORIENTATION_180 = 2,
  PI_ILI_ORIENTATION_270 = 3,
} pi_ili_orientation_e;


typedef enum
{
  PI_ILI_IOCTL_ORIENTATION = PI_DISPLAY_IOCTL_CUSTOM
} pi_ili_ioctl_cmd_e;



/** \brief Initialize a camera configuration with default values.
 *
 * The structure containing the configuration must be kept alive until the camera device is opened.
 * Can only be called from fabric-controller side.
 *
 * \param conf A pointer to the camera configuration.
 */
void pi_ili9341_conf_init(struct pi_ili9341_conf *conf);



#endif 