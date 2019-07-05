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

#ifndef __BSP__RAM__HYPERRAM_H__
#define __BSP__RAM__HYPERRAM_H__

#include "bsp/ram.h"

struct hyperram_conf
{
  struct ram_conf ram;
  int hyper_itf;
  int hyper_cs;
  char skip_pads_config;
  int ram_start;
  int ram_size;
};




/** \brief Initialize an Hyperflash configuration with default values.
 *
 * The structure containing the configuration must be kept alive until the hyperflash device is opened.
 *
 * \param conf A pointer to the hyperflash configuration.
 */
void hyperram_conf_init(struct hyperram_conf *conf);



#endif 