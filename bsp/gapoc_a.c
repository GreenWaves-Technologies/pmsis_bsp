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

#include "bsp/gapoc_a.h"

#include "bsp/camera/mt9v034.h"
#include "bsp/flash/hyperflash.h"
#include "bsp/transport/nina_w10.h"
#include "bsp/display/ili9341.h"
#include "bsp/ram/hyperram.h"


void bsp_hyperram_conf_init(struct hyperram_conf *conf)
{
  conf->ram_start = CONFIG_HYPERRAM_START;
  conf->ram_size = CONFIG_HYPERRAM_SIZE;
  conf->skip_pads_config = 0;
  conf->hyper_itf = CONFIG_HYPERRAM_HYPER_ITF;
  conf->hyper_cs = CONFIG_HYPERRAM_HYPER_CS;
}

int bsp_hyperram_open(struct hyperram_conf *conf)
{
  return 0;
}



void bsp_hyperflash_conf_init(struct hyperflash_conf *conf)
{
  conf->hyper_itf = CONFIG_HYPERFLASH_HYPER_ITF;
  conf->hyper_cs = CONFIG_HYPERFLASH_HYPER_CS;
}

int bsp_hyperflash_open(struct hyperflash_conf *conf)
{
  return 0;
}



void bsp_mt9v034_conf_init(struct mt9v034_conf *conf)
{
  conf->i2c_itf = CONFIG_MT9V034_I2C_ITF;
  conf->cpi_itf = CONFIG_MT9V034_CPI_ITF;
  conf->power_gpio = CONFIG_MT9V034_POWER_GPIO;
  conf->trigger_gpio = CONFIG_MT9V034_TRIGGER_GPIO;
}

int bsp_mt9v034_open(struct mt9v034_conf *conf)
{
  if (!conf->skip_pads_config)
  {
    rt_pad_set_function(CONFIG_MT9V034_TRIGGER_GPIO_PAD, CONFIG_MT9V034_TRIGGER_GPIO_PAD_FUNC);

    rt_pad_set_function(CONFIG_MT9V034_POWER_GPIO_PAD, CONFIG_MT9V034_POWER_GPIO_PAD_FUNC);
  }

  return 0;
}



void bsp_nina_w10_conf_init(struct nina_w10_conf *conf)
{
  conf->spi_itf = CONFIG_NINA_W10_SPI_ITF;
  conf->spi_cs = CONFIG_NINA_W10_SPI_CS;
}

int bsp_nina_w10_open(struct nina_w10_conf *conf)
{
  return 0;
}


void bsp_ili9341_conf_init(struct ili9341_conf *conf)
{
  conf->gpio = CONFIG_ILI9341_GPIO;
  conf->spi_itf = CONFIG_ILI9341_SPI_ITF;
  conf->spi_cs = CONFIG_ILI9341_SPI_CS;

}

int bsp_ili9341_open(struct ili9341_conf *conf)
{
  if (!conf->skip_pads_config)
  {
 #ifndef __ZEPHYR__
    rt_pad_set_function(CONFIG_ILI9341_GPIO_PAD, CONFIG_ILI9341_GPIO_PAD_FUNC);
 #endif
  }

  return 0;
}
