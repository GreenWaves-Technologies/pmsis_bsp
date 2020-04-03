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
 * Authors: Yao ZHANG, GreenWaves Technologies (yao.zhang@greenwaves-technologies.com)
 *          Francesco PACI, GreenWaves Technologies (francesco.paci@greenwaves-technologies.com)
            Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#include "pmsis.h"
#include "pmsis/rtos/os_frontend_api/pmsis_time.h"
#include "bsp/bsp.h"
#include "bsp/camera/ov7670.h"
#include "ov7670.h"


static ov7670_reg_init_t __ov7670_reg_init[] =
{
    {0x40, 0xd0},
    {0x12, 0x00},
    {0x0c, 0x00},
    {0x3e, 0x00},
    {0x70, 0x00},
    {0x71, 0x00},
    {0x72, 0x11},
    {0x73, 0x00},
    {0xa2, 0x02},
    {0x11, 0x81},
    {0x7a, 0x20},
    {0x7b, 0x1c},
    {0x7c, 0x28},
    {0x7d, 0x3c},
    {0x7e, 0x55},
    {0x7f, 0x68},
    {0x80, 0x76},
    {0x81, 0x80},
    {0x82, 0x88},
    {0x83, 0x8f},
    {0x84, 0x96},
    {0x85, 0xa3},
    {0x86, 0xaf},
    {0x87, 0xc4},
    {0x88, 0xd7},
    {0x89, 0xe8},
    {0x13, 0xe0},
    {0x00, 0x00},
    {0x10, 0x00},
    {0x0d, 0x00},
    {0x14, 0x28},
    {0xa5, 0x05},
    {0xab, 0x07},
    {0x24, 0x75},
    {0x25, 0x63},
    {0x26, 0xA5},
    {0x9f, 0x78},
    {0xa0, 0x68},
    {0xa1, 0x03},
    {0xa6, 0xdf},
    {0xa7, 0xdf},
    {0xa8, 0xf0},
    {0xa9, 0x90},
    {0xaa, 0x94},
    {0x13, 0xe5},
    {0x0e, 0x61},
    {0x0f, 0x4b},
    {0x16, 0x02},
    {0x1e, 0x17},
    {0x21, 0x02},
    {0x22, 0x91},
    {0x29, 0x07},
    {0x33, 0x0b},
    {0x35, 0x0b},
    {0x37, 0x1d},
    {0x38, 0x71},
    {0x39, 0x2a},
    {0x3c, 0x78},
    {0x4d, 0x40},
    {0x4e, 0x20},
    {0x69, 0x00},
    {0x6b, 0x00},
    {0x74, 0x19},
    {0x8d, 0x4f},
    {0x8e, 0x00},
    {0x8f, 0x00},
    {0x90, 0x00},
    {0x91, 0x00},
    {0x92, 0x00},
    {0x96, 0x00},
    {0x9a, 0x80},
    {0xb0, 0x84},
    {0xb1, 0x0c},
    {0xb2, 0x0e},
    {0xb3, 0x82},
    {0xb8, 0x0a},
    {0x43, 0x14},
    {0x44, 0xf0},
    {0x45, 0x34},
    {0x46, 0x58},
    {0x47, 0x28},
    {0x48, 0x3a},
    {0x59, 0x88},
    {0x5a, 0x88},
    {0x5b, 0x44},
    {0x5c, 0x67},
    {0x5d, 0x49},
    {0x5e, 0x0e},
    {0x64, 0x04},
    {0x65, 0x20},
    {0x66, 0x05},
    {0x94, 0x04},
    {0x95, 0x08},
    {0x6c, 0x0a},
    {0x6d, 0x55},
    {0x6e, 0x11},
    {0x6f, 0x9f},
    {0x6a, 0x40},
    {0x01, 0x40},
    {0x02, 0x40},
    {0x13, 0xe7},
    {0x15, 0x02},
    {0x4f, 0x80},
    {0x50, 0x80},
    {0x51, 0x00},
    {0x52, 0x22},
    {0x53, 0x5e},
    {0x54, 0x80},
    {0x58, 0x9e},
    {0x41, 0x08},
    {0x3f, 0x00},
    {0x75, 0x05},
    {0x76, 0xe1},
    {0x4c, 0x00},
    {0x77, 0x01},
    {0x3d, 0xc2},
    {0x4b, 0x09},
    {0xc9, 0x60},
    {0x41, 0x38},
    {0x56, 0x40},
    {0x34, 0x11},
    {0x3b, 0x02},
    {0xa4, 0x89},
    {0x96, 0x00},
    {0x97, 0x30},
    {0x98, 0x20},
    {0x99, 0x30},
    {0x9a, 0x84},
    {0x9b, 0x29},
    {0x9c, 0x03},
    {0x9d, 0x4c},
    {0x9e, 0x3f},
    {0x78, 0x04},
    {0x79, 0x01},
    {0xc8, 0xf0},
    {0x79, 0x0f},
    {0xc8, 0x00},
    {0x79, 0x10},
    {0xc8, 0x7e},
    {0x79, 0x0a},
    {0xc8, 0x80},
    {0x79, 0x0b},
    {0xc8, 0x01},
    {0x79, 0x0c},
    {0xc8, 0x0f},
    {0x79, 0x0d},
    {0xc8, 0x20},
    {0x79, 0x09},
    {0xc8, 0x80},
    {0x79, 0x02},
    {0xc8, 0xc0},
    {0x79, 0x03},
    {0xc8, 0x40},
    {0x79, 0x05},
    {0xc8, 0x30},
    {0x79, 0x26},
    {0x09, 0x03},
    {0x3b, 0x42},
//    {0xff, 0xff}
};

static inline int is_i2c_active()
{
#if defined(ARCHI_PLATFORM_RTL)

  // I2C driver is not yet working on some chips, at least this works on gvsoc.
  // Also there is noI2C connection to camera model on RTL
#if PULP_CHIP == CHIP_GAP9 || PULP_CHIP == CHIP_VEGA || PULP_CHIP == CHIP_ARNOLD || PULP_CHIP == CHIP_PULPISSIMO || PULP_CHIP == CHIP_PULPISSIMO_V1
  return 0;
#else
  return rt_platform() != ARCHI_PLATFORM_RTL;
#endif

#else

  return 1;

#endif
}



static void __ov7670_reg_write(ov7670_t *ov7670, unsigned int addr, uint8_t value)
{
  if (is_i2c_active())
  {
    ov7670->i2c_req.value = value;
    ov7670->i2c_req.addr = (uint8_t)addr;
    pi_i2c_write(&ov7670->i2c_device, (uint8_t *)&ov7670->i2c_req, 2, PI_I2C_XFER_STOP);
  }
}



static uint8_t __ov7670_reg_read(ov7670_t *ov7670, unsigned int addr)
{
  if (is_i2c_active())
  {
    ov7670->i2c_req.addr = (addr & 0xFF);
    pi_i2c_write(&ov7670->i2c_device, (uint8_t *)&ov7670->i2c_req, 1, PI_I2C_XFER_STOP);
    pi_i2c_read(&ov7670->i2c_device, (uint8_t *)&ov7670->i2c_read_value, 1, PI_I2C_XFER_STOP);
    return *(volatile uint8_t *)&ov7670->i2c_read_value;
  }
  else
  {
    return 0;
  }
}



static void __ov7670_init_regs(ov7670_t *ov7670)
{
    unsigned int i;
    for(i=0; i<(sizeof(__ov7670_reg_init)/sizeof(ov7670_reg_init_t)); i++)
    {
        __ov7670_reg_write(ov7670, __ov7670_reg_init[i].addr, __ov7670_reg_init[i].value);
    }
}



static void __ov7670_reset(ov7670_t *ov7670)
{
    __ov7670_reg_write(ov7670, REG_COM7, COM7_RESET);
    pi_time_wait_us(1000);
}


static void __ov7670_mode(ov7670_t *ov7670, uint8_t mode)
{
    uint8_t value=0;
    value = __ov7670_reg_read(ov7670, REG_COM2);
    if (mode == OV7670_STREAMING)
        value &= (~COM2_SSLEEP&0xFF);
    else if (mode == OV7670_STANDBY)
        value |= (COM2_SSLEEP&0xFF);

    __ov7670_reg_write(ov7670, REG_COM2, value);
}



static void __ov7670_wakeup(ov7670_t *ov7670)
{
  if (!ov7670->is_awake)
  {
    __ov7670_mode(ov7670, OV7670_STREAMING);
    ov7670->is_awake = 1;
  }
}



static void __ov7670_standby(ov7670_t *ov7670)
{
  if (ov7670->is_awake)
  {
    __ov7670_mode(ov7670, OV7670_STANDBY);
    ov7670->is_awake = 0;
  }
}



int32_t __ov7670_open(struct pi_device *device)
{
    struct pi_ov7670_conf *conf = (struct pi_ov7670_conf *)device->config;

    ov7670_t *ov7670 = (ov7670_t *)pmsis_l2_malloc(sizeof(ov7670_t));
    if (ov7670 == NULL) return -1;

    device->data = (void *)ov7670;

    if (bsp_ov7670_open(conf))
        goto error;

    struct pi_cpi_conf cpi_conf;
    pi_cpi_conf_init(&cpi_conf);
    cpi_conf.itf = conf->cpi_itf;
    pi_open_from_conf(&ov7670->cpi_device, &cpi_conf);

    if (pi_cpi_open(&ov7670->cpi_device))
        goto error;

    struct pi_i2c_conf i2c_conf;
    pi_i2c_conf_init(&i2c_conf);
    i2c_conf.cs = 0x42;
    i2c_conf.itf = conf->i2c_itf;
    i2c_conf.max_baudrate = 200000;
    pi_open_from_conf(&ov7670->i2c_device, &i2c_conf);

    if (pi_i2c_open(&ov7670->i2c_device))
        goto error2;

    pi_cpi_set_format(&ov7670->cpi_device, PI_CPI_FORMAT_BYPASS_BIGEND);

    __ov7670_reset(ov7670);
    __ov7670_init_regs(ov7670);

    return 0;

error2:
    pi_cpi_close(&ov7670->cpi_device);
error:
    pmsis_l2_malloc_free(ov7670, sizeof(ov7670_t));
    return -1;
}



static void __ov7670_close(struct pi_device *device)
{
    ov7670_t *ov7670 = (ov7670_t *)device->data;
    __ov7670_standby(ov7670);
    pi_cpi_close(&ov7670->cpi_device);
    pmsis_l2_malloc_free(ov7670, sizeof(ov7670_t));
}



static int32_t __ov7670_control(struct pi_device *device, pi_camera_cmd_e cmd, void *arg)
{
    int irq = disable_irq();

    ov7670_t *ov7670 = (ov7670_t *)device->data;

    switch (cmd)
    {
        case PI_CAMERA_CMD_ON:
            __ov7670_wakeup(ov7670);
            break;

        case PI_CAMERA_CMD_OFF:
            __ov7670_standby(ov7670);
            break;

        case PI_CAMERA_CMD_START:
            pi_cpi_control_start(&ov7670->cpi_device);
            break;

        case PI_CAMERA_CMD_STOP:
            pi_cpi_control_stop(&ov7670->cpi_device);
            break;

        default:
            break;
    }

    restore_irq(irq);

    return 0;
}

pi_task_t task_cpi = {0};

void __pi_cpi_cb(void *arg)
{
	ov7670_callbak * ov_callbak = (ov7670_callbak*)arg;
	pi_task_t *task = ov_callbak->task;
	void * buffer = ov_callbak->buffer;
	uint32_t bufferlen = ov_callbak->bufferlen;

	pi_cpi_capture_async(&ov_callbak->ov7670->cpi_device, buffer+(128*1024), (bufferlen-128*1024-1), task);
}

void __ov7670_capture_async(struct pi_device *device, void *buffer, uint32_t bufferlen, pi_task_t *task)
{
    ov7670_t *ov7670 = (ov7670_t *)device->data;

	ov7670_callbak ov_callbak = {0};
	ov_callbak.ov7670 = ov7670;
	ov_callbak.task = task;
	ov_callbak.buffer = buffer;
	ov_callbak.bufferlen = bufferlen;

  if(bufferlen < (128*1024)){
	pi_cpi_capture_async(&ov7670->cpi_device, buffer,bufferlen , task);
  }
  else
  {
      pi_task_callback(&task_cpi, __pi_cpi_cb, (void *)&ov_callbak);
 		pi_cpi_capture_async(&ov7670->cpi_device, buffer, (128*1024-1), &task_cpi);
  }
}



int32_t __ov7670_reg_set(struct pi_device *device, uint32_t addr, uint8_t *value)
{
    ov7670_t *ov7670 = (ov7670_t *)device->data;
    __ov7670_reg_write(ov7670, addr, *value);
    return 0;
}



int32_t __ov7670_reg_get(struct pi_device *device, uint32_t addr, uint8_t *value)
{
    ov7670_t *ov7670 = (ov7670_t *)device->data;
    *value = __ov7670_reg_read(ov7670, addr);
    return 0;
}



static pi_camera_api_t ov7670_api =
{
    .open           = &__ov7670_open,
    .close          = &__ov7670_close,
    .control        = &__ov7670_control,
    .capture_async  = &__ov7670_capture_async,
    .reg_set        = &__ov7670_reg_set,
    .reg_get        = &__ov7670_reg_get
};



void pi_ov7670_conf_init(struct pi_ov7670_conf *conf)
{
    conf->camera.api = &ov7670_api;
    conf->skip_pads_config = 0;
    bsp_ov7670_conf_init(conf);
    __camera_conf_init(&conf->camera);
}
