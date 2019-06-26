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
 * Authors:  Esteban Gougeon, GreenWaves Technologies (esteban.gougeon@greenwaves-technologies.com)
 *           Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#include "pmsis.h"
#include "bsp/transport/nina_w10.h"
#include "bsp/bsp.h"
#include "bsp/transport.h"


typedef struct
{
  struct pi_device spim;
} nina_t;


int __nina_w10_open(struct pi_device *device)
{
  struct nina_w10_conf *conf = (struct nina_w10_conf *)device->config;

  nina_t *nina = (nina_t *)pmsis_l2_malloc(sizeof(nina_t));
  if (nina == NULL) return -1;

  struct pi_spi_conf spi_conf;
  pi_spi_conf_init(&spi_conf);
  spi_conf.itf = conf->spi_itf;
  spi_conf.cs = conf->spi_cs;

  spi_conf.wordsize = PI_SPI_WORDSIZE_8;
  spi_conf.big_endian = 1;
  spi_conf.max_baudrate = 50000000;
  spi_conf.polarity = 0;
  spi_conf.phase = 0;
  
  pi_open_from_conf(&nina->spim, &spi_conf);

  if (pi_spi_open(&nina->spim))
    goto error;

  device->data = (void *)nina;

  return 0;

error:
  pmsis_l2_malloc_free(nina, sizeof(nina_t));
  return -1;
}



int __nina_w10_connect(struct pi_device *device, int channel, void (*rcv_callback(void *arg, struct transport_header)), void *arg)
{
  return 0;
}



void __nina_w10_close(struct pi_device *device)
{

}



int __nina_w10_send_async(struct pi_device *device, void *buffer, size_t size, pi_task_t *task)
{
  nina_t *nina = (nina_t *)device->data;

  pi_spi_send_async(&nina->spim, buffer, size*8, PI_SPI_CS_AUTO, task);
  return 0;
}



int __nina_w10_receive_async(struct pi_device *device, void *buffer, size_t size, pi_task_t *task)
{
  return 0;
}



static transport_api_t nina_w10_api = 
{
  .open              = &__nina_w10_open,
  .connect           = &__nina_w10_connect,
  .send_async        = &__nina_w10_send_async,
  .receive_async     = &__nina_w10_receive_async,
  .close             = &__nina_w10_close,
};



void nina_w10_conf_init(struct nina_w10_conf *conf)
{
  conf->transport.api = &nina_w10_api;
  bsp_nina_w10_conf_init(conf);
}
