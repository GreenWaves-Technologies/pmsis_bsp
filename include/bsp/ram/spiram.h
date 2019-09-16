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

#ifndef __BSP__RAM__SPIRAM_H__
#define __BSP__RAM__SPIRAM_H__

#include "bsp/ram.h"

struct spiram_conf
{
  struct ram_conf ram;
  int spi_itf;
  int spi_cs;
  char skip_pads_config;
  int ram_start;
  int ram_size;
  uint32_t baudrate;
};




/** \brief Initialize an SPI ram configuration with default values.
 *
 * The structure containing the configuration must be kept alive until the SPI ram device is opened.
 *
 * \param conf A pointer to the SPI ram configuration.
 */
void spiram_conf_init(struct spiram_conf *conf);



#endif 