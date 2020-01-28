/*
 * Copyright (C) 2018 GreenWaves Technologies
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
 * Authors: Mathieu Barbe, GreenWaves Technologies (mathieu.barbe@greenwaves-technologies.com)
 */

#include "string.h"
#include "stdio.h"

#include "pmsis.h"
#include "bsp/bsp.h"
#include "bsp/flash.h"
#include "bsp/partition.h"

#include "flash_partition.h"

static const pi_partition_info_t *partition_table = NULL;
static pmsis_mutex_t *partition_table_mutex;


size_t pi_partition_get_size(const pi_partition_t *partition)
{
    if (partition)
        return partition->size;
    else
        return 0;
}

uint32_t pi_partition_get_flash_offset(const pi_partition_t *partition)
{
    if (partition)
        return partition->offset;
    else
        return UINT32_MAX;
}

void print_partition_table(const pi_partition_info_t *table)
{
    if (table == NULL)
    {
        printf("No partition table\n");
        return;
    }

    printf("## Label \t   Type Sub Type Offset Length\n");

    for (uint8_t i = 0;
         table[i].magic_bytes == PI_PARTITION_MAGIC;
         i++)
    {
        printf("%2d %-16s 0x%02x 0x%02x 0x%-8lx 0x%-8lx\n",
               i, table[i].label,
               table[i].type, table[i].subtype,
               table[i].pos.offset, table[i].pos.size);
    }
}

static pi_err_t ensure_partitions_loaded(pi_device_t *flash)
{
    pi_err_t rc;

    if (partition_table)
        return PI_OK;

    //    Check if partition table mutex is initialized
    if (!partition_table_mutex)
    {
        int irq_enabled;
        hal_compiler_barrier();
        irq_enabled = disable_irq();
        hal_compiler_barrier();
        partition_table_mutex = pmsis_l2_malloc(sizeof(pmsis_mutex_t));
        if (partition_table_mutex == NULL)
            return PI_ERR_NO_MEM;
        if (pmsis_mutex_init(partition_table_mutex))
            return PI_FAIL;
        restore_irq(irq_enabled);
        hal_compiler_barrier();
    }

    // only lock if the partition table is empty (and check again after acquiring lock)
    pmsis_mutex_take(partition_table_mutex);
    if (partition_table)
    {
        pmsis_mutex_release(partition_table_mutex);
        return PI_OK;
    }

    rc = pi_partition_table_load(flash, &partition_table, NULL);
    pmsis_mutex_release(partition_table_mutex);

    print_partition_table(partition_table);

    return rc;
}

const pi_partition_info_t *_pi_partition_find_first(pi_partition_type_t type,
                                              pi_partition_subtype_t subtype, const char *label)
{
    const pi_partition_info_t *part;

    if (partition_table == NULL)
    {
        return NULL;
    }

    for (part = partition_table; part->magic_bytes != PI_PARTITION_MAGIC; part++)
    {
        if (part->type != type || part->subtype != subtype)
            continue;
        if (label == NULL)
            break;
        if (strncmp(label, (char *) &part->label, PI_PARTITION_LABEL_LENGTH))
            continue;
    }
    return part;
}



const pi_partition_t *
pi_partition_find_first(pi_device_t *flash, const pi_partition_type_t type, const pi_partition_subtype_t subtype, const char *label)
{
	pi_err_t rc;
	const pi_partition_info_t *info;
	pi_partition_t *partition;
	
	puts("In find partition");
	
	rc = ensure_partitions_loaded(flash);
	if (rc != PI_OK)
	{
		return NULL;
	}
	
	info = _pi_partition_find_first(type, subtype, label);
	if (info == NULL)
		return NULL;
	
	partition = pi_l2_malloc(sizeof(pi_partition_t));
	partition->type = info->type;
	partition->subtype = info->subtype;
	partition->size = info->pos.size;
	partition->offset = info->pos.offset;
	memcpy(partition->label, info->label, 16);
	partition->label[16] = 0;
	partition->encrypted = false;
	partition->read_only = false;
	
	return (const pi_partition_t *) partition;
	
}
