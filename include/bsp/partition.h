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

#ifndef __BSP_PARTITION_H__
#define __BSP_PARTITION_H__

#include "pmsis.h"
#include "bsp/flash.h"

/**
 * @defgroup Partition Partition
 *
 * The partition driver provides support for handling partition
 * contained into a device storage.
 *
 */

/**
 * @addtogroup Partition
 * @{
 */

/**@{*/

/** \enum pi_partition_type_e
 * \brief Partition type
 *
 */
typedef enum {
    PI_PARTITION_RAW_TYPE = 0,
    PI_PARTITION_BINARY_TYPE = 1,
    PI_PARTITION_READFS_TYPE = 2,
    PI_PARTITION_LFS_TYPE = 3,
} pi_partition_type_e;

typedef struct partition {
    struct pi_device *flash;
    pi_partition_type_e type;
    uint32_t offset;
    uint32_t size;
} pi_partition_t;


/** \struct pi_partition_conf
 * \brief Partition configuration structure.
 *
 */
struct pi_partition_conf {
    uint8_t id;
    pi_device_t *flash;
};

/** \brief Open a partition device.
 *
 * This function must be called before the partition can be used.
 * It will do all the needed configuration to make it usable and initialize
 * the handle used to refer to this opened device when calling other functions.
 *
 * \param device    A pointer to the device structure of the device to open.
 *   This structure is allocated by the called and must be kept alive until the
 *   device is closed.
 * \return          0 if the operation is successfull, -1 if there was an error.
 */
int pi_partition_open(struct pi_device *device);

//!@}

/**
 * @} end of Partition
 */

/// @cond IMPLEM

static inline int pi_partition_close(struct pi_device *device)
{
    pi_l2_free(device->data, sizeof(pi_partition_t));
    return 0;
}

#define CHECK_ADDR() if (partition_addr + size > partition->size) return -1

static inline int pi_partition_read_async(struct pi_device *device, const uint32_t partition_addr,
                                          void *data, const size_t size, pi_task_t *task)
{
    pi_partition_t *partition = (pi_partition_t *) device->data;
    struct pi_partition_conf *conf = (struct pi_partition_conf *) device->config;

    CHECK_ADDR();
    pi_flash_read_async(conf->flash, partition_addr + partition->offset, data, size, task);
    return 0;
}

static inline int pi_partition_read(struct pi_device *device, const uint32_t partition_addr,
                                    void *data, const size_t size)
{
    int rc;
    pi_task_t task;

    pi_task_block(&task);
    rc = pi_partition_read_async(device, partition_addr, data, size, &task);
    if (rc < 0)
        return rc;
    pi_task_wait_on(&task);
    return 0;
}

static inline int pi_partition_write_async(struct pi_device *device, const uint32_t partition_addr, const void *data,
                                           const size_t size, pi_task_t *task)
{
    pi_partition_t *partition = (pi_partition_t *) device->data;
    struct pi_partition_conf *conf = (struct pi_partition_conf *) device->config;

    CHECK_ADDR();
    pi_flash_program_async(conf->flash, partition_addr + partition->offset, data, size, task);
    return 0;
}

static inline int
pi_partition_write(struct pi_device *device, const uint32_t partition_addr, const void *data, const size_t size)
{
    int rc;
    pi_task_t task;

    pi_task_block(&task);
    rc = pi_partition_write_async(device, partition_addr, data, size, &task);
    if (rc < 0)
        return rc;
    pi_task_wait_on(&task);
    return 0;
}

static inline int pi_partition_erase_async(struct pi_device *device, uint32_t partition_addr, int size, pi_task_t *task)
{
    pi_partition_t *partition = (pi_partition_t *) device->data;
    struct pi_partition_conf *conf = (struct pi_partition_conf *) device->config;

    CHECK_ADDR();
    pi_flash_erase_async(conf->flash, partition_addr + partition->offset, size, task);
    return 0;
}

static inline int pi_partition_erase(struct pi_device *device, uint32_t partition_addr, int size)
{
    int rc;
    pi_task_t task;

    pi_task_block(&task);
    rc = pi_partition_erase_async(device, partition_addr, size, &task);
    if (rc < 0)
        return rc;
    pi_task_wait_on(&task);
    return 0;
}

static inline int pi_partition_erase_partition_async(struct pi_device *device, pi_task_t *task)
{
    pi_partition_t *partition = (pi_partition_t *) device->data;

    return pi_partition_erase_async(device, 0, partition->size, task);
}

static inline int pi_partition_erase_partition(struct pi_device *device)
{
    int rc;
    pi_task_t task;

    pi_task_block(&task);
    rc = pi_partition_erase_partition_async(device, &task);
    if (rc < 0)
        return 0;
    pi_task_wait_on(&task);
    return 0;
}

size_t pi_partition_get_size(pi_device_t *device);

uint32_t pi_partition_get_flash_offset(pi_device_t *device);

/// @endcond

#endif
