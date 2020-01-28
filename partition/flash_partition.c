/*
 * Copyright (C) 2020 GreenWaves Technologies
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
 * Created by Mathieu Barbe <mathieu.barbe@greenwaves-technologies.com>.
 * on 1/6/2020.
 */

#include "string.h"
#include "stdio.h"

#include "flash_partition.h"
#include "md5.h"

static void print_partition_header(pi_partition_table_header_t *header)
{
    printf("Partition table header:\n"
           "\t.magic_bytes 0x%04x\n"
           "\t.format_version %u\n"
           "\t.nbr_of_entries %u\n"
           "\t.crc_flag %u\n",
           header->magic_bytes,
           header->format_version,
           header->nbr_of_entries,
           header->crc_flags);
    printf("\t.md5 ");
    for (int i = 0; i < 16; i++)
    {
        printf("%02x ", header->md5[i]);
    }
    printf("\n");
}

pi_err_t pi_partition_table_verify(const pi_partition_table_header_t *header,
                                   const pi_partition_info_t *partition_table)
{
    const pi_partition_info_t *part;
    MD5_CTX context;
    uint8_t digest[16];

    // Check magic number for each partition
    for (uint8_t num_parts = 0; num_parts < header->nbr_of_entries; num_parts++)
    {
        part = partition_table + num_parts;
        if (part->magic_bytes != PI_PARTITION_MAGIC)
            return PI_ERR_INVALID_STATE;
    }

    // Check if last entry is empty
    part = partition_table + header->nbr_of_entries;
    if (part->magic_bytes == PI_PARTITION_MAGIC)
        return PI_ERR_INVALID_STATE;

    if (header->crc_flags)
    {
        MD5_Init(&context);
        MD5_Update(&context, (unsigned char *) partition_table, header->nbr_of_entries * sizeof(pi_partition_info_t));
        MD5_Final(digest, &context);

        if (strncmp((const char *) header->md5, (const char *) digest, sizeof(digest)))
        {
            return PI_ERR_INVALID_CRC;
        }
    }

    return PI_OK;
}


pi_err_t pi_partition_table_load(pi_device_t *flash, const pi_partition_info_t **partition_table, uint8_t *nbr_of_entries)
{
    pi_err_t rc = PI_OK;
    uint32_t *table_offset = NULL;
    pi_partition_table_header_t *header = NULL;
    pi_partition_info_t *table = NULL;

    if (partition_table == NULL)
        return PI_ERR_INVALID_ARG;

    table_offset = pi_l2_malloc(sizeof(*table_offset));
    if (table_offset == NULL)
    {
        rc = PI_ERR_L2_NO_MEM;
        goto _return;
    }

    pi_flash_read(flash, 0, table_offset, 4);
    if (*table_offset == 0)
    {
        rc = PI_ERR_NOT_FOUND;
        goto _return;
    }

    header = pi_l2_malloc(sizeof(*header));
    if (header == NULL)
    {
        rc = PI_ERR_L2_NO_MEM;
        goto _return;
    }

    pi_flash_read(flash, *table_offset, header, sizeof(*header));

    print_partition_header(header);

    if (header->format_version != PI_PARTITION_TABLE_FORMAT_VERSION)
    {
        printf("Partition table format version missmatch: flash version %u != BSP version %u\n", header->format_version,
               PI_PARTITION_TABLE_FORMAT_VERSION);
        rc = PI_ERR_INVALID_VERSION;
        goto _return;
    }

    if (header->magic_bytes != PI_PARTITION_TABLE_HEADER_MAGIC)
    {
        printf("Partition table header magic number error\n");
        rc = PI_ERR_NOT_FOUND;
        goto _return;
    }

    table = pi_l2_malloc(sizeof(pi_partition_info_t) * (header->nbr_of_entries + 1));
    if (table == NULL)
    {
        rc = PI_ERR_L2_NO_MEM;
        goto _return;
    }

    pi_flash_read(flash, *table_offset + PI_PARTITION_HEADER_SIZE, table,
                  sizeof(pi_partition_info_t) * header->nbr_of_entries);
    memset(table + header->nbr_of_entries, 0, sizeof(pi_partition_info_t));

    if (header->crc_flags)
    {
        rc = pi_partition_table_verify(header, table);
        if (rc != PI_OK)
        {
            printf("Partitions table verification failed.\n");
            goto _return;
        }
    }

    *partition_table = table;
    if (nbr_of_entries)
        *nbr_of_entries = header->nbr_of_entries;

    _return:
    if (table_offset)
        pi_l2_free(table_offset, sizeof(*table_offset));
    if (header)
        pi_l2_free(header, sizeof(*header));
    return rc;
}
