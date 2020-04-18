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
#include "bsp/camera/ov5640.h"
#include "ov5640.h"

typedef struct {
  uint16_t addr;
  uint8_t value;
} i2c_req_t;


typedef struct {
  struct pi_device cpi_device;
  struct pi_device i2c_device;

  struct pi_device ov5640_reset;
  struct pi_device ov5640_pwdn;

  i2c_req_t i2c_req;
  uint32_t i2c_read_value;

  int is_awake;
} ov5640_t;


typedef i2c_req_t ov5640_reg_init_t;


static ov5640_reg_init_t __ov5640_reg_init[] = {} ;

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



static void __ov5640_reg_write(ov5640_t *ov5640, uint16_t addr, uint8_t value)
{
  if (is_i2c_active())
  {
    ov5640->i2c_req.value = value;
    ov5640->i2c_req.addr = ((addr >> 8) & 0xff) | ((addr & 0xff) << 8);
    pi_i2c_write(&ov5640->i2c_device, (uint8_t *)&ov5640->i2c_req, 3, PI_I2C_XFER_STOP);
  }
}



static uint8_t __ov5640_reg_read(ov5640_t *ov5640, uint16_t addr)
{
  if (is_i2c_active())
  {
    ov5640->i2c_req.addr = ((addr >> 8) & 0xff) | ((addr & 0xff) << 8);
    pi_i2c_write(&ov5640->i2c_device, (uint8_t *)&ov5640->i2c_req, 2, PI_I2C_XFER_STOP);
    pi_i2c_read(&ov5640->i2c_device, (uint8_t *)&ov5640->i2c_read_value, 1, PI_I2C_XFER_STOP);
    return *(volatile uint8_t *)&ov5640->i2c_read_value;
  }
  else
  {
    return 0;
  }
}



static void __ov5640_init_regs(ov5640_t *ov5640)
{
    unsigned int i;
    for(i=0; i<(sizeof(__ov5640_reg_init)/sizeof(ov5640_reg_init_t)); i++)
    {
        __ov5640_reg_write(ov5640, __ov5640_reg_init[i].addr, __ov5640_reg_init[i].value);
    }
}



static void __ov5640_reset(ov5640_t *ov5640)
{
    //HW reset: pull the RESETB pin to 0
    // SW reset
    __ov5640_reg_write(ov5640, 0x3008, 0x80);
    // Datasheet: Wait 1ms after reset
    pi_time_wait_us(1000);
}


static void __ov5640_mode(ov5640_t *ov5640, uint8_t mode)
{
    uint8_t value=0;
    value = __ov5640_reg_read(ov5640, 0x3008);
    if (mode == OV5640_STREAMING)
        value &= 0xBF;
    else if (mode == OV5640_STANDBY)
        value |= 0x40;

    __ov5640_reg_write(ov5640, REG_COM2, value);
}



static void __ov5640_wakeup(ov5640_t *ov5640)
{
  if (!ov5640->is_awake)
  {
    __ov5640_mode(ov5640, OV5640_STREAMING);
    ov5640->is_awake = 1;
  }
}



static void __ov5640_standby(ov5640_t *ov5640)
{
  if (ov5640->is_awake)
  {
    __ov5640_mode(ov5640, OV5640_STANDBY);
    ov5640->is_awake = 0;
  }
}



int32_t __ov5640_open(struct pi_device *device)
{
    struct pi_ov5640_conf *conf = (struct pi_ov5640_conf *)device->config;

    ov5640_t *ov5640 = (ov5640_t *)pmsis_l2_malloc(sizeof(ov5640_t));
    if (ov5640 == NULL) return -1;

    device->data = (void *)ov5640;

    if (bsp_ov5640_open(conf))
        goto error;

    struct pi_cpi_conf cpi_conf;
    pi_cpi_conf_init(&cpi_conf);
    cpi_conf.itf = conf->cpi_itf;
    pi_open_from_conf(&ov5640->cpi_device, &cpi_conf);

    if (pi_cpi_open(&ov5640->cpi_device))
        goto error;

    struct pi_i2c_conf i2c_conf;
    pi_i2c_conf_init(&i2c_conf);
    i2c_conf.cs = 0x78;
    i2c_conf.itf = conf->i2c_itf;
    i2c_conf.max_baudrate = 200000;
    pi_open_from_conf(&ov5640->i2c_device, &i2c_conf);

    if (pi_i2c_open(&ov5640->i2c_device))
        goto error2;

    pi_cpi_set_format(&ov5640->cpi_device, PI_CPI_FORMAT_BYPASS_BIGEND);

    __ov5640_reset(ov5640);
    __ov5640_init_regs(ov5640);

    return 0;

error2:
    pi_cpi_close(&ov5640->cpi_device);
error:
    pmsis_l2_malloc_free(ov5640, sizeof(ov5640_t));
    return -1;
}



static void __ov5640_close(struct pi_device *device)
{
    ov5640_t *ov5640 = (ov5640_t *)device->data;
    __ov5640_standby(ov5640);
    pi_cpi_close(&ov5640->cpi_device);
    pmsis_l2_malloc_free(ov5640, sizeof(ov5640_t));
}



static int32_t __ov5640_control(struct pi_device *device, pi_camera_cmd_e cmd, void *arg)
{
    int irq = disable_irq();

    ov5640_t *ov5640 = (ov5640_t *)device->data;

    switch (cmd)
    {
        case PI_CAMERA_CMD_ON:
            __ov5640_wakeup(ov5640);
            break;

        case PI_CAMERA_CMD_OFF:
            __ov5640_standby(ov5640);
            break;

        case PI_CAMERA_CMD_START:
            pi_cpi_control_start(&ov5640->cpi_device);
            break;

        case PI_CAMERA_CMD_STOP:
            pi_cpi_control_stop(&ov5640->cpi_device);
            break;

        default:
            break;
    }

    restore_irq(irq);

    return 0;
}

void __ov5640_capture_async(struct pi_device *device, void *buffer, uint32_t bufferlen, pi_task_t *task)
{
    ov5640_t *ov5640 = (ov5640_t *)device->data;

	pi_cpi_capture_async(&ov5640->cpi_device, buffer, bufferlen, task);
}



int32_t __ov5640_reg_set(struct pi_device *device, uint32_t addr, uint8_t *value)
{
    ov5640_t *ov5640 = (ov5640_t *)device->data;
    __ov5640_reg_write(ov5640, addr, *value);
    return 0;
}



int32_t __ov5640_reg_get(struct pi_device *device, uint32_t addr, uint8_t *value)
{
    ov5640_t *ov5640 = (ov5640_t *)device->data;
    *value = __ov5640_reg_read(ov5640, addr);
    return 0;
}



static pi_camera_api_t ov5640_api =
{
    .open           = &__ov5640_open,
    .close          = &__ov5640_close,
    .control        = &__ov5640_control,
    .capture_async  = &__ov5640_capture_async,
    .reg_set        = &__ov5640_reg_set,
    .reg_get        = &__ov5640_reg_get
};



void pi_ov5640_conf_init(struct pi_ov5640_conf *conf)
{
    conf->camera.api = &ov5640_api;
    conf->skip_pads_config = 0;
    bsp_ov5640_conf_init(conf);
    __camera_conf_init(&conf->camera);
}
