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
#include "bsp/bsp.h"
#include "bsp/ram/spiram.h"
#include "pmsis/drivers/spi.h"
#include "../extern_alloc.h"


typedef struct
{
  struct pi_device spi_device;
  extern_alloc_t alloc;
  uint32_t *send_ucode;
  uint32_t *receive_ucode;
  uint32_t buffer;
} spiram_t;



static int __spiram_send_cmd(spiram_t *spiram, uint32_t cmd, uint32_t flags)
{
  spiram->buffer = cmd;
  pi_spi_send(&spiram->spi_device, (uint8_t *)&spiram->buffer, 8, flags);
  return 0;
}



static int spiram_open(struct pi_device *device)
{
  struct spiram_conf *conf = (struct spiram_conf *)device->config;

  spiram_t *spiram = (spiram_t *)pmsis_l2_malloc(sizeof(spiram_t));
  if (spiram == NULL)
  {
      return -1;
  }

  device->data = (void *)spiram;

  if (extern_alloc_init(&spiram->alloc, 0, conf->ram_size))
  {
      goto error;
  }

  if (bsp_spiram_open(conf))
  {
      goto error2;
  }

  struct pi_spi_conf spi_conf;
  pi_spi_conf_init(&spi_conf);

  spi_conf.itf = conf->spi_itf;
  spi_conf.cs = conf->spi_cs;
  if (conf->baudrate)
  {
      spi_conf.max_baudrate = conf->baudrate;
  }

  pi_open_from_conf(&spiram->spi_device, &spi_conf);

  int32_t error = pi_spi_open(&spiram->spi_device);
  if (error)
  {
      goto error2;
  }


    // SAFE_PADCFG8 for D2 , A11, B10, A10  -  HIGH drive, pull disabled
    #define SAFE_PADCFG8    ((uint32_t*)0x1A1041A0)
    *SAFE_PADCFG8 = 0x02020202;
    // SAFE_PADCFG9 for B8, A8, B7, (A9)-  HIGH drive, pull disabled
    #define SAFE_PADCFG9    ((uint32_t*)0x1A1041A4)
    *SAFE_PADCFG9 = 0x02020202;

  //rt_time_wait_us(100000);
  __spiram_send_cmd(spiram, 0x66, PI_SPI_CS_AUTO | PI_SPI_LINES_QUAD);
  //rt_time_wait_us(100000);
  __spiram_send_cmd(spiram, 0x99, PI_SPI_CS_AUTO | PI_SPI_LINES_QUAD);
  //rt_time_wait_us(100000);
  __spiram_send_cmd(spiram, 0x35, PI_SPI_CS_AUTO);
  //rt_time_wait_us(100000);

  uint32_t ucode[4];

  ucode[0] = SPI_UCODE_CMD_SEND_CMD(0x38, 8, 1);
  ucode[1] = SPI_UCODE_CMD_SEND_ADDR(24, 1);

  spiram->send_ucode = pi_spi_send_ucode_set(&spiram->spi_device, (uint8_t *)ucode, 3*4);
  pi_spi_send_ucode_set_addr_info(&spiram->spi_device, ((uint8_t *)spiram->receive_ucode) + 2*4 + 1, 3);

  ucode[0] = SPI_UCODE_CMD_SEND_CMD(0xEB, 8, 1);
  ucode[1] = SPI_UCODE_CMD_SEND_ADDR(24, 1);
  ucode[3] = SPI_CMD_DUMMY(6);

  spiram->receive_ucode = pi_spi_receive_ucode_set(&spiram->spi_device, (uint8_t *)ucode, 4*4);
  pi_spi_receive_ucode_set_addr_info(&spiram->spi_device, ((uint8_t *)spiram->receive_ucode) + 2*4 + 1, 3);

  return 0;

error2:
  extern_alloc_init(&spiram->alloc, 0, conf->ram_size);
error:
  pmsis_l2_malloc_free(spiram, sizeof(spiram_t));
  return -2;
}



static void spiram_close(struct pi_device *device)
{
  spiram_t *spiram = (spiram_t *)device->data;
  pi_spi_close(&spiram->spi_device);
  pmsis_l2_malloc_free(spiram, sizeof(spiram_t));
}



static void spiram_copy_async(struct pi_device *device, uint32_t addr, void *data, uint32_t size, int ext2loc, pi_task_t *task)
{
  spiram_t *spiram = (spiram_t *)device->data;

  if (ext2loc)
  {
    spiram->receive_ucode[2] = addr << 8;
    pi_spi_receive_async(&spiram->spi_device, data, size*8, PI_SPI_APPEND_UCODE | PI_SPI_CS_AUTO | PI_SPI_LINES_QUAD, task);
  }
  else
  {
    spiram->send_ucode[2] = addr << 8;
    pi_spi_send_async(&spiram->spi_device, data, size*8, PI_SPI_APPEND_UCODE | PI_SPI_CS_AUTO | PI_SPI_LINES_QUAD, task);
  }
}



static void spiram_copy_2d_async(struct pi_device *device, uint32_t addr, void *data, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, pi_task_t *task)
{
  spiram_t *spiram = (spiram_t *)device->data;

#if 0
  if (ext2loc)
    pi_spi_read_2d_async(&spiram->spi_device, addr, data, size, stride, length, task);
  else
    pi_spi_write_2d_async(&spiram->spi_device, addr, data, size, stride, length, task);
#endif
}




int spiram_alloc(struct pi_device *device, uint32_t *addr, uint32_t size)
{
  void *chunk;
  spiram_t *spiram = (spiram_t *)device->data;
  int err = extern_alloc(&spiram->alloc, size, &chunk);
  *addr = (uint32_t)chunk;
  return err;
}



int spiram_free(struct pi_device *device, uint32_t addr, uint32_t size)
{
  spiram_t *spiram = (spiram_t *)device->data;
  return extern_free(&spiram->alloc, size, (void *)addr);
}


#if 0

void __pi_spiram_alloc_cluster_req(void *_req)
{
  pi_cl_spiram_alloc_req_t *req = (pi_cl_spiram_alloc_req_t *)_req;
  req->result = pi_spiram_alloc(req->device, req->size);
  req->done = 1;
  __pi_cluster_notif_req_done(req->cid);
}



void __pi_spiram_free_cluster_req(void *_req)
{
  pi_cl_spiram_free_req_t *req = (pi_cl_spiram_free_req_t *)_req;
  pi_spiram_free(req->device, req->chunk, req->size);
  req->done = 1;
  __pi_cluster_notif_req_done(req->cid);
}



void pi_cl_spiram_alloc(struct pi_device *device, uint32_t size, pi_cl_spiram_alloc_req_t *req)
{
  req->device = device;
  req->size = size;
  req->cid = pi_cluster_id();
  req->done = 0;
  __pi_task_init_from_cluster(&req->event);
  pi_task_callback(&req->event, __pi_spiram_alloc_cluster_req, (void* )req);
  __pi_cluster_push_fc_event(&req->event);
}



void pi_cl_spiram_free(struct pi_device *device, uint32_t chunk, uint32_t size, pi_cl_spiram_free_req_t *req)
{
  req->device = device;
  req->size = size;
  req->chunk = chunk;
  req->cid = pi_cluster_id();
  req->done = 0;
  __pi_task_init_from_cluster(&req->event);
  pi_task_callback(&req->event, __pi_spiram_free_cluster_req, (void* )req);
  __pi_cluster_push_fc_event(&req->event);
}

#endif

static ram_api_t spiram_api = {
  .open                 = &spiram_open,
  .close                = &spiram_close,
  .copy_async           = &spiram_copy_async,
  .copy_2d_async        = &spiram_copy_2d_async,
  .alloc                = &spiram_alloc,
  .free                 = &spiram_free,
};


void spiram_conf_init(struct spiram_conf *conf)
{
  conf->ram.api = &spiram_api;
  conf->baudrate = 0;
  bsp_spiram_conf_init(conf);
}

