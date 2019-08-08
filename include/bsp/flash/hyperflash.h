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


#ifndef __BSP__FLASH__HYPERFLASH_H__
#define __BSP__FLASH__HYPERFLASH_H__

#include "bsp/flash.h"

struct hyperflash_conf
{
  struct flash_conf flash;
  int hyper_itf;
  int hyper_cs;
  char skip_pads_config;
  flash_type_e type;
};




/** \brief Initialize an Hyperflash configuration with default values.
 *
 * The structure containing the configuration must be kept alive until the hyperflash device is opened.
 *
 * \param conf A pointer to the hyperflash configuration.
 */
void hyperflash_conf_init(struct hyperflash_conf *conf);



#endif 
