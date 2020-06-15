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
#include "bsp/eeprom/24xx1025.h"

typedef struct
{
    struct pi_device i2c_device;
    pi_task_t *waiting_first;
    pi_task_t *waiting_last;
    pi_task_t *pending_task;
} eeprom_t;


int pi_24xx1025_open(struct pi_device *device)
{
    struct pi_24xx1025_conf *conf = (struct pi_24xx1025_conf *)device->config;

    eeprom_t *eeprom = pi_l2_malloc(sizeof(eeprom_t));
    if (eeprom == NULL)
        return -1;

    device->data = (void *)eeprom;

    struct pi_i2c_conf i2c_conf;
    pi_i2c_conf_init(&i2c_conf);
    i2c_conf.cs = conf->i2c_addr;
    i2c_conf.itf = conf->i2c_itf;
    pi_open_from_conf(&eeprom->i2c_device, &i2c_conf);

    if (pi_i2c_open(&eeprom->i2c_device))
        goto error;

    eeprom->pending_task = NULL;
    eeprom->waiting_first = NULL;

error:
    pmsis_l2_malloc_free(eeprom, sizeof(eeprom_t));
    return -1;
}


static void pi_24xx1025_close(struct pi_device *device)
{

}


static void bsp_24xx1025_enqueue_copy(eeprom_t *eeprom, uint32_t eeprom_addr, void *data, uint32_t size, pi_task_t *task, int is_write)
{
    if (!eeprom->pending_task)
    {
        if (is_write)
            pi_i2c_write_dual_async(&eeprom->i2c_device, &eeprom_addr, data, 2, size, task);
        else
            pi_i2c_write_read_async(&eeprom->i2c_device, &eeprom_addr, data, 2, size, task);
    }
    else
    {
        task->data[0] = (int)eeprom;
        task->data[1] = (int)data;
        task->data[2] = size;
        task->data[3] = is_write;

        if (eeprom->waiting_first)
            eeprom->waiting_last->next = task;
        else
            eeprom->waiting_first = task;
    
        eeprom->waiting_last = task;
        task->next = NULL;
    }
}


static void pi_24xx1025_read_async(struct pi_device *device, uint32_t eeprom_addr, void *data, uint32_t size, pi_task_t *task)
{
    bsp_24xx1025_enqueue_copy(device->data, eeprom_addr, data, size, task, 0);
}


static void pi_24xx1025_write_async(struct pi_device *device, uint32_t eeprom_addr, void *data, uint32_t size, pi_task_t *task)
{
    bsp_24xx1025_enqueue_copy(device->data, eeprom_addr, data, size, task, 1);
}


static pi_eeprom_api_t pi_24xx1025_api =
{
  .open                 = &pi_24xx1025_open,
  .close                = &pi_24xx1025_close,
  .read_async           = &pi_24xx1025_read_async,
  .write_async          = &pi_24xx1025_write_async
};


void pi_24xx1025_conf_init(struct pi_24xx1025_conf *conf)
{
  conf->eeprom.api = &pi_24xx1025_api;
  bsp_24xx1025_conf_init(conf);
}