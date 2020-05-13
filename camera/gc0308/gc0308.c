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
#include "bsp/camera/gc0308.h"
#include "gc0308.h"

typedef struct {
    uint8_t addr;
    uint8_t value;
} i2c_req_t;


typedef struct {
    struct pi_gc0308_conf conf;
    struct pi_device cpi_device;
    struct pi_device i2c_device;

    struct pi_device gc0308_reset;

    i2c_req_t i2c_req;
    uint32_t i2c_read_value;

    int is_awake;
} gc0308_t;


typedef i2c_req_t gc0308_reg_init_t;


static gc0308_reg_init_t __gc0308_reg_init[] =
{

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



static void __gc0308_reg_write(gc0308_t *gc0308, uint8_t addr, uint8_t value)
{
    if (is_i2c_active())
    {
        gc0308->i2c_req.value = value;
        gc0308->i2c_req.addr = (uint8_t)addr;
        pi_i2c_write(&gc0308->i2c_device, (uint8_t *)&gc0308->i2c_req, 2, PI_I2C_XFER_STOP);
    }
}



static uint8_t __gc0308_reg_read(gc0308_t *gc0308, uint8_t addr)
{
    if (is_i2c_active())
    {
        gc0308->i2c_req.addr = (addr & 0xFF);
        pi_i2c_write(&gc0308->i2c_device, (uint8_t *)&gc0308->i2c_req.addr, 1, PI_I2C_XFER_NO_STOP);
        pi_i2c_read(&gc0308->i2c_device, (uint8_t *)&gc0308->i2c_req.value, 1, PI_I2C_XFER_STOP);
        return gc0308->i2c_req.value;
    }
    return 0;
}



static void __gc0308_init_regs(gc0308_t *gc0308)
{
    int32_t i;
    for(i=0; i<(int32_t)(sizeof(__gc0308_reg_init)/sizeof(gc0308_reg_init_t)); i++)
    {
        __gc0308_reg_write(gc0308, __gc0308_reg_init[i].addr, __gc0308_reg_init[i].value);
    }
}



static void __gc0308_reset(gc0308_t *gc0308)
{
    pi_gpio_pin_write(&gc0308->gc0308_reset, gc0308->conf.reset_gpio, 0);
    pi_time_wait_us(1000);
    pi_gpio_pin_write(&gc0308->gc0308_reset, gc0308->conf.reset_gpio, 1);
    pi_time_wait_us(1000);
}


static void __gc0308_mode(gc0308_t *gc0308, uint8_t mode)
{
    // The camera standby mode is controlled by PWDN pin, which is not connected on GAPuino board.
    // Do nothing here
}



static void __gc0308_wakeup(gc0308_t *gc0308)
{
    if (!gc0308->is_awake)
    {
        // The camera standby mode is controlled by PWDN pin, which is not connected on GAPuino board.
        // Do nothing here
        gc0308->is_awake = 1;
    }
}



static void __gc0308_standby(gc0308_t *gc0308)
{
    if (gc0308->is_awake)
    {
        // The camera standby mode is controlled by PWDN pin, which is not connected on GAPuino board.
        // Do nothing here
        gc0308->is_awake = 0;
    }
}



int32_t __gc0308_open(struct pi_device *device)
{
    struct pi_gc0308_conf *conf = (struct pi_gc0308_conf *)device->config;

    gc0308_t *gc0308 = (gc0308_t *)pmsis_l2_malloc(sizeof(gc0308_t));
    if (gc0308 == NULL) return -1;

    device->data = (void *)gc0308;

    if (bsp_gc0308_open(conf))
        goto error;

    struct pi_gpio_conf gpio_reset_conf;
    pi_gpio_conf_init(&gpio_reset_conf);
    pi_open_from_conf(&gc0308->gc0308_reset, &gpio_reset_conf);
    if (pi_gpio_open(&gc0308->gc0308_reset))
        goto error;

    pi_gpio_pin_configure(&gc0308->gc0308_reset, conf->reset_gpio, PI_GPIO_OUTPUT);

    struct pi_cpi_conf cpi_conf;
    pi_cpi_conf_init(&cpi_conf);
    cpi_conf.itf = conf->cpi_itf;
    pi_open_from_conf(&gc0308->cpi_device, &cpi_conf);

    if (pi_cpi_open(&gc0308->cpi_device))
        goto error;

    struct pi_i2c_conf i2c_conf;
    pi_i2c_conf_init(&i2c_conf);
    i2c_conf.cs = 0x42;
    i2c_conf.itf = conf->i2c_itf;
    i2c_conf.max_baudrate = 200000;
    pi_open_from_conf(&gc0308->i2c_device, &i2c_conf);
    printf("i2c : %d\n", i2c_conf.itf);

    if (pi_i2c_open(&gc0308->i2c_device))
        goto error2;


    // Workaround for FreeRTOS, TODO to be fixed
#if defined(__FREERTOS__)
    pi_cpi_set_format(&gc0308->cpi_device, PI_CPI_FORMAT_BYPASS_LITEND);
#else
    pi_cpi_set_format(&gc0308->cpi_device, PI_CPI_FORMAT_BYPASS_BIGEND);
#endif

    //__gc0308_reset(gc0308);
    __gc0308_init_regs(gc0308);

    return 0;

error2:
    pi_cpi_close(&gc0308->cpi_device);
error:
    pmsis_l2_malloc_free(gc0308, sizeof(gc0308_t));
    return -1;
}



static void __gc0308_close(struct pi_device *device)
{
    gc0308_t *gc0308 = (gc0308_t *)device->data;
    __gc0308_standby(gc0308);
    pi_cpi_close(&gc0308->cpi_device);
    pmsis_l2_malloc_free(gc0308, sizeof(gc0308_t));
}



static int32_t __gc0308_control(struct pi_device *device, pi_camera_cmd_e cmd, void *arg)
{
    int irq = disable_irq();

    gc0308_t *gc0308 = (gc0308_t *)device->data;

    switch (cmd)
    {
        case PI_CAMERA_CMD_ON:
            __gc0308_wakeup(gc0308);
            break;

        case PI_CAMERA_CMD_OFF:
            __gc0308_standby(gc0308);
            break;

        case PI_CAMERA_CMD_START:
            pi_cpi_control_start(&gc0308->cpi_device);
            break;

        case PI_CAMERA_CMD_STOP:
            pi_cpi_control_stop(&gc0308->cpi_device);
            break;

        default:
            break;
    }

    restore_irq(irq);

    return 0;
}

void __gc0308_capture_async(struct pi_device *device, void *buffer, uint32_t bufferlen, pi_task_t *task)
{
    gc0308_t *gc0308 = (gc0308_t *)device->data;

    pi_cpi_capture_async(&gc0308->cpi_device, buffer, bufferlen, task);
}



int32_t __gc0308_reg_set(struct pi_device *device, uint32_t addr, uint8_t *value)
{
    gc0308_t *gc0308 = (gc0308_t *)device->data;
    __gc0308_reg_write(gc0308, addr, *value);
    return 0;
}



int32_t __gc0308_reg_get(struct pi_device *device, uint32_t addr, uint8_t *value)
{
    gc0308_t *gc0308 = (gc0308_t *)device->data;
    *value = __gc0308_reg_read(gc0308, addr);
    return 0;
}



static pi_camera_api_t gc0308_api =
{
    .open           = &__gc0308_open,
    .close          = &__gc0308_close,
    .control        = &__gc0308_control,
    .capture_async  = &__gc0308_capture_async,
    .reg_set        = &__gc0308_reg_set,
    .reg_get        = &__gc0308_reg_get
};



void pi_gc0308_conf_init(struct pi_gc0308_conf *conf)
{
    conf->camera.api = &gc0308_api;
    conf->skip_pads_config = 0;
    bsp_gc0308_conf_init(conf);
    __camera_conf_init(&conf->camera);
}
