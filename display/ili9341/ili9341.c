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
 * Authors: Francesco Paci, GreenWaves Technologies (francesco.paci@greenwaves-technologies.com)
 *          Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */


#include "pmsis.h"
#include "bsp/display/ili9341.h"
#include "bsp/bsp.h"
#include "ili9341.h"

#define TEMP_BUFFER_SIZE 256



typedef struct
{
  struct pi_device spim;
  int width;
  uint32_t current_data;
  uint32_t current_size;
  uint32_t current_line_len;
  pi_buffer_t *buffer;
  uint8_t temp_buffer[TEMP_BUFFER_SIZE*2];
  pi_task_t temp_copy_task;
  pi_task_t *current_task;
  int gpio;
} ili_t;



static void __ili_init(ili_t *ili);
static void __ili_set_rotation(ili_t *ili,uint8_t m);
static void __ili_set_addr_window(ili_t *ili,uint16_t x, uint16_t y, uint16_t w, uint16_t h) ;
static void __ili_write_8(ili_t *ili, uint8_t  d);
static void __ili_write_command(ili_t *ili,uint8_t cmd);
static void __ili_gray8_to_rgb565(uint8_t *input,uint16_t *output,int width, int height);


static void __ili9341_write_buffer_iter(void *arg)
{
  ili_t *ili = (ili_t *)arg;
  uint32_t size = TEMP_BUFFER_SIZE;

  if (ili->buffer->stride != 0)
  {
    if (size > ili->current_line_len)
      size = ili->current_line_len;
  }
  else
  {
    if (size > ili->current_size)
      size = ili->current_size;
  }

  pi_task_t *task = &ili->temp_copy_task;
  pi_spi_flags_e flags = PI_SPI_CS_KEEP;

  ili->current_size -= size;
  if (ili->current_size == 0)
  {
    task = ili->current_task;
    flags = PI_SPI_CS_AUTO;
  }

  __ili_gray8_to_rgb565((uint8_t *)ili->current_data, (uint16_t *)ili->temp_buffer, size, 1);

  pi_spi_send_async(&ili->spim, ili->temp_buffer, size*2*8, flags, task);

  ili->current_data += size;

  if (ili->buffer->stride != 0)
  {
    ili->current_line_len -= size;
    if (ili->current_line_len == 0)
    {
      ili->current_line_len = ili->width;
      ili->current_data += ili->buffer->stride;
    }
  }
}



static void __ili_write_async(struct pi_device *device, pi_buffer_t *buffer, uint16_t x, uint16_t y,uint16_t w, uint16_t h, pi_task_t *task)
{
  ili_t *ili = (ili_t *)device->data;

  __rt_task_init(task);

  __ili_set_addr_window(ili, x, y, w, h); // Clipped area

  pi_task_callback(&ili->temp_copy_task, __ili9341_write_buffer_iter, (void *)ili);

  ili->buffer = buffer;
  ili->width = w;
  ili->current_task = task;
  ili->current_data = (uint32_t)buffer->data;
  ili->current_size = w*h;
  ili->current_line_len = w;

  __ili9341_write_buffer_iter(ili);
}



static int __ili_open(struct pi_device *device)
{
  struct ili9341_conf *conf = (struct ili9341_conf *)device->config;
  int periph_id;
  int channel;

  ili_t *ili = (ili_t *)pmsis_l2_malloc(sizeof(ili_t));
  if (ili == NULL) return -1;

  if (bsp_ili9341_open(conf))
    goto error;

  device->data = (void *)ili;

  ili->gpio = conf->gpio;

  struct pi_spi_conf spi_conf;
  pi_spi_conf_init(&spi_conf);
  spi_conf.itf = conf->spi_itf;
  spi_conf.cs = conf->spi_cs;

  spi_conf.wordsize = PI_SPI_WORDSIZE_8;
  spi_conf.big_endian = 1;
  spi_conf.max_baudrate = 50000000;
  spi_conf.polarity = 0;
  spi_conf.phase = 0;
  
  pi_open_from_conf(&ili->spim, &spi_conf);

  if (pi_spi_open(&ili->spim))
    goto error;

  __ili_init(ili);
  __ili_set_rotation(ili,3);
  
  return 0;

error:
  pmsis_l2_malloc_free(ili, sizeof(ili_t));
  return -1;
}


static display_api_t ili_api = 
{
  .open           = &__ili_open,
  .write_async    = &__ili_write_async,
};



void ili9341_conf_init(struct ili9341_conf *conf)
{
  conf->display.api = &ili_api;
  conf->spi_itf = 0;
  conf->skip_pads_config = 0;
  __display_conf_init(&conf->display);
  bsp_ili9341_conf_init(conf);
}



static void __ili_write_8(ili_t *ili, uint8_t value) 
{
  ili->temp_buffer[0] = value;
  pi_spi_send(&ili->spim, ili->temp_buffer, 8, PI_SPI_CS_AUTO);
}



static void __ili_write_16(ili_t *ili, uint16_t value) 
{
  __ili_write_8(ili, value >> 8);
  __ili_write_8(ili, value);
}



static void __ili_write_32(ili_t *ili, uint16_t value) 
{
  __ili_write_16(ili, value >> 16);
  __ili_write_16(ili, value);
}



static void __ili_write_command(ili_t *ili, uint8_t cmd)
{
  rt_gpio_set_pin_value(0, ili->gpio, 0);
  __ili_write_8(ili,cmd);
  rt_gpio_set_pin_value(0, ili->gpio, 1);
}



static void __ili_init(ili_t *ili)
{
  rt_gpio_init(0, ili->gpio);    
  rt_gpio_set_dir(0, 1<<ili->gpio, RT_GPIO_IS_OUT);
  rt_gpio_set_pin_value(0, ili->gpio, 0);

  __ili_write_command(ili,0xEF);
  __ili_write_8(ili,0x03);
  __ili_write_8(ili,0x80);
  __ili_write_8(ili,0x02);

  __ili_write_command(ili,0xCF);
  __ili_write_8(ili,0x00);
  __ili_write_8(ili,0XC1);
  __ili_write_8(ili,0X30);

  __ili_write_command(ili,0xED);
  __ili_write_8(ili,0x64);
  __ili_write_8(ili,0x03);
  __ili_write_8(ili,0X12);
  __ili_write_8(ili,0X81);

  __ili_write_command(ili, 0xE8);
  __ili_write_8(ili,0x85);
  __ili_write_8(ili,0x00);
  __ili_write_8(ili,0x78);

  __ili_write_command(ili,0xCB);
  __ili_write_8(ili,0x39);
  __ili_write_8(ili,0x2C);
  __ili_write_8(ili,0x00);
  __ili_write_8(ili,0x34);
  __ili_write_8(ili,0x02);

  __ili_write_command(ili,0xF7);
  __ili_write_8(ili,0x20);

  __ili_write_command(ili,0xEA);
  __ili_write_8(ili,0x00);
  __ili_write_8(ili,0x00);

  __ili_write_command(ili,ILI9341_PWCTR1);    //Power control
  __ili_write_8(ili,0x23);   //VRH[5:0]

  __ili_write_command(ili,ILI9341_PWCTR2);    //Power control
  __ili_write_8(ili,0x10);   //SAP[2:0];BT[3:0]

  __ili_write_command(ili,ILI9341_VMCTR1);    //VCM control
  __ili_write_8(ili,0x3e);
  __ili_write_8(ili,0x28);

  __ili_write_command(ili,ILI9341_VMCTR2);    //VCM control2
  __ili_write_8(ili,0x86);  //--

  __ili_write_command(ili,ILI9341_MADCTL);    // Memory Access Control
  __ili_write_8(ili,0x48);

  __ili_write_command(ili,ILI9341_VSCRSADD); // Vertical scroll
  __ili_write_16(ili,0);                 // Zero

  __ili_write_command(ili,ILI9341_PIXFMT);
  __ili_write_8(ili,0x55);

  __ili_write_command(ili,ILI9341_FRMCTR1);
  __ili_write_8(ili,0x00);
  __ili_write_8(ili,0x18);

  __ili_write_command(ili,ILI9341_DFUNCTR);    // Display Function Control
  __ili_write_8(ili,0x08);
  __ili_write_8(ili,0x82);
  __ili_write_8(ili,0x27);

  __ili_write_command(ili,0xF2);    // 3Gamma Function Disable
  __ili_write_8(ili,0x00);

  __ili_write_command(ili,ILI9341_GAMMASET);    //Gamma curve selected
  __ili_write_8(ili,0x01);

  __ili_write_command(ili,ILI9341_GMCTRP1);    //Set Gamma
  __ili_write_8(ili,0x0F);
  __ili_write_8(ili,0x31);
  __ili_write_8(ili,0x2B);
  __ili_write_8(ili,0x0C);
  __ili_write_8(ili,0x0E);
  __ili_write_8(ili,0x08);
  __ili_write_8(ili,0x4E);
  __ili_write_8(ili,0xF1);
  __ili_write_8(ili,0x37);
  __ili_write_8(ili,0x07);
  __ili_write_8(ili,0x10);
  __ili_write_8(ili,0x03);
  __ili_write_8(ili,0x0E);
  __ili_write_8(ili,0x09);
  __ili_write_8(ili,0x00);

  __ili_write_command(ili,ILI9341_GMCTRN1);    //Set Gamma
  __ili_write_8(ili,0x00);
  __ili_write_8(ili,0x0E);
  __ili_write_8(ili,0x14);
  __ili_write_8(ili,0x03);
  __ili_write_8(ili,0x11);
  __ili_write_8(ili,0x07);
  __ili_write_8(ili,0x31);
  __ili_write_8(ili,0xC1);
  __ili_write_8(ili,0x48);
  __ili_write_8(ili,0x08);
  __ili_write_8(ili,0x0F);
  __ili_write_8(ili,0x0C);
  __ili_write_8(ili,0x31);
  __ili_write_8(ili,0x36);
  __ili_write_8(ili,0x0F);

  __ili_write_command(ili,ILI9341_SLPOUT);    //Exit Sleep
  rt_time_wait_us(120000);
  __ili_write_command(ili,ILI9341_DISPON);    //Display on
  rt_time_wait_us(120000);
}



static void __ili_set_rotation(ili_t *ili, uint8_t m)
{
  int rotation = m % 4; // can't be higher than 3
  switch (rotation)
  {
    case 0:
      m = (MADCTL_MX | MADCTL_BGR);
      break;

    case 1:
      m = (MADCTL_MV | MADCTL_BGR);
      break;

    case 2:
      m = (MADCTL_MY | MADCTL_BGR);
      break;

    case 3:
      m = (MADCTL_MX | MADCTL_MY | MADCTL_MV | MADCTL_BGR);
      break;
  }

  __ili_write_command(ili, ILI9341_MADCTL);
  __ili_write_8(ili, m);
}



static void __ili_gray8_to_rgb565(uint8_t *input,uint16_t *output,int width, int height)
{
  for(int i=0;i<width*height;i++)
  {
    output[i] = ((input[i] >> 3 ) << 3) | ((input[i] >> 5) ) | (((input[i] >> 2 ) << 13) )|   ((input[i] >> 3) <<8);
  }
}



static void __ili_set_addr_window(ili_t *ili,uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
  uint32_t xa = ((uint32_t)x << 16) | (x+w-1);
  uint32_t ya = ((uint32_t)y << 16) | (y+h-1);
  __ili_write_command(ili,ILI9341_CASET); // Column addr set
  __ili_write_32(ili,xa);
  __ili_write_command(ili,ILI9341_PASET); // Row addr set
  __ili_write_32(ili,ya);
  __ili_write_command(ili,ILI9341_RAMWR); // write to RAM
}
