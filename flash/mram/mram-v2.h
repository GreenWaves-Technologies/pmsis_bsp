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
 * Authors: Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#include "pmsis.h"

#include "archi/chips/gap9_v2/pulp_archi.h"


#ifndef __DRIVERS__MRAM__MRAM_V2_H__
#define __DRIVERS__MRAM__MRAM_V2_H__


#if !defined(__TRACE_ALL__) && !defined(__TRACE_MRAM__)
#define MRAM_TRACE(x...)
#else
#define MRAM_TRACE(level, x...) POS_TRACE(level, "[MRAM] " x)
#endif


typedef struct
{
    uint32_t base;
    pi_task_t *pending_copy;
    pi_task_t *waiting_first;
    pi_task_t *waiting_last;
    int freq;
    uint8_t tx_channel;
    uint8_t rx_channel;
    uint8_t id;
    uint8_t open_count;
    uint8_t periph_id;
} pos_mram_t;




extern PI_FC_L1 pos_mram_t pos_mram[ARCHI_UDMA_NB_MRAM];


static inline void __attribute__((always_inline)) pos_mram_copy_2d_exec(pos_mram_t *mram, uint32_t addr, uint32_t size, uint32_t mram_addr, uint32_t stride, uint32_t length, int is_read)
{
    udma_mram_trans_addr_set(mram->base, (uint32_t)addr);
    udma_mram_trans_size_set(mram->base, size);
    udma_mram_ext_addr_set(mram->base, mram_addr);
    udma_mram_line_2d_set(mram->base, length);
    udma_mram_stride_2d_set(mram->base, stride);
    udma_mram_trans_cfg_set(mram->base, UDMA_MRAM_TRANS_CFG_RXTX(is_read) | UDMA_MRAM_TRANS_CFG_VALID(1));
}


#endif