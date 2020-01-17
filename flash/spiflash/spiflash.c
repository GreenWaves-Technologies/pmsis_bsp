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
 * Authors: Antoine Faravelon, GreenWaves Technologies 
 * (antoine.faravelon@greenwaves-technologies.com)
 */


/*
 * TODO 1: Mutexes on all functions using shared vars
 * TODO 2: re uniformize api
 * TODO 3: Port to new spi API (depend on implem new api for spi)
 */
#include "pmsis.h"
#include "bsp/bsp.h"
#include "pmsis/drivers/spi.h"

#define SECTOR_SIZE (1<<12)
#define QSPI_FLASH_CS 0

#define QSPIF_DUMMY_CYCLES(x)   ((uint8_t)((x&0xF)<<3))
#define QSPIF_BURST_LEN(x)      ((uint8_t)((x&0x3)<<0))
#define QSPIF_BURST_ENA(x)      ((uint8_t)((x&0x1)<<1))

#define QSPIF_WR_READ_REG_CMD   ((uint8_t)0xC0)
#define QSPIF_WR_EN_CMD         ((uint8_t)0x06)
#define QSPIF_QPI_EN_CMD        ((uint8_t)0x35)
#define QSPIF_QPI_FAST_READ_CMD ((uint8_t)0x0b)
#define QSPIF_QIO_FAST_READ_CMD ((uint8_t)0xEB)
#define QSPIF_QO_FAST_READ_CMD  ((uint8_t)0x6B)
#define QSPIF_PAGE_PROG_CMD     ((uint8_t)0x02)
#define QSPIF_QIO_PAGE_PROG_CMD ((uint8_t)0x32)
#define QSPIF_ERASE_SECTOR_CMD  ((uint8_t)0x20)
#define QSPIF_READ_STATUS_CMD   ((uint8_t)0x05)

#define QSPIF_WR_PROLOGUE_SIZE 0x4
#define QSPIF_RCV_UCODE_SIZE   (36)
#define QSPIF_PPQ_UCODE_SIZE   (5*sizeof(uint32_t))
#define QSPIF_ERASE_SIZE       0x4
#define QSPIF_PAGE_SIZE        0x100

// TODO: replace by conf
#define QE_STATUS_ID        0
#define QE_BIT_POS          6
#define READ_QE_STATUS_CMD  0x5
#define WRITE_STATUS_QE_CMD 0x1
#define STATUS_REG_SIZE     1
#define BUSY_BIT_ID         0
#define BUSY_BIT_POS        0

#define IS_BUSY(status_reg) ((status_reg[BUSY_BIT_ID] >> BUSY_BIT_POS) & 0x1)

// DUMMY Cycles defined for typical 50MHz settings
#define DUMMY_CYCLES 3
// Avoid QPI problems
#define SPI_FLASH_USE_QUAD_IO

#ifndef SPI_FLASH_USE_QUAD_IO 
#define SPI_LINES_FLAG PI_SPI_LINES_QUAD
#else
#define SPI_LINES_FLAG PI_SPI_LINES_SINGLE
#endif

typedef struct {
  struct pi_device qspi_dev;
  struct pi_spi_conf qspi_conf;
  size_t sector_size;
  // Used for communications with hyperflash through udma
  uint32_t ucode_buffer[9];
  uint32_t cmd_buf[2];
  uint32_t status_reg[(STATUS_REG_SIZE/4)+1];
  pi_mutex_t flash_op_mutex;

  // Waiting queue for common operations (only 1 is handled at the same time)
  pi_task_t *waiting_first;
  pi_task_t *waiting_last;

  // Task to be enqueued when the on-going operation is done
  pi_task_t *pending_task;
} spi_flash_t;


// ------ globals for some regular operations
static const uint8_t g_set_qspif_dummy[] = {QSPIF_WR_READ_REG_CMD,
    QSPIF_DUMMY_CYCLES(DUMMY_CYCLES)};
static const uint8_t g_enter_qpi_mode[] = {0x35,0};
static const uint8_t g_exit_qpi_mode[]  = {0xf5,0};
static const uint8_t g_chip_erase[]     = {0x60,0};
static const uint8_t g_write_enable[]   = {QSPIF_WR_EN_CMD,0};


static void wait_wip(spi_flash_t *flash_dev);

static inline void qpi_flash_set_quad_enable(spi_flash_t *flash_dev)
{
    volatile uint8_t* cmd_buf = (uint8_t*)flash_dev->cmd_buf;
    uint8_t* status_reg = (uint8_t*)flash_dev->status_reg;
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;
    // read status register, and then write it back with QE bit = 1
    cmd_buf[0] = 0x5;READ_QE_STATUS_CMD; // read status reg
    pi_spi_send(qspi_dev, (void*)&cmd_buf[0], 1*8,
            PI_SPI_LINES_SINGLE | PI_SPI_CS_KEEP);
    pi_spi_receive(qspi_dev, (void*)&status_reg[QE_STATUS_ID], 1*8,
            PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
    if(!((status_reg[QE_STATUS_ID] >> QE_BIT_POS)&1))
    {
        status_reg[QE_STATUS_ID] |= (1 << QE_BIT_POS);
        // WREN before WRSR
        cmd_buf[0] = QSPIF_WR_EN_CMD;
        pi_spi_send(qspi_dev, (void*)cmd_buf, 1*8,
                PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
        cmd_buf[0] = WRITE_STATUS_QE_CMD; // write status reg (holds QE bit)
        cmd_buf[1] = status_reg[QE_STATUS_ID];
        pi_spi_send(qspi_dev, (void*)cmd_buf, 2*8,
                PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
        // flash takes some time to recover from QE bit/status reg write
        pi_time_wait_us(100000);
    }
}

static inline void qpi_flash_pre_config(spi_flash_t *flash_dev)
{
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;
#ifndef SPI_FLASH_USE_QUAD_IO
    // Enter QPI mode
#warn "QSPI enter is compiled -- boot from spi flash might be compromised"
    pi_spi_send(qspi_dev, (void*)g_enter_qpi_mode, 1*8,
            PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO);
#endif
#ifdef FLASH_CHIP_ERASE
    // flash chip erase (optional!)
    pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
            SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    pi_spi_send(qspi_dev, (void*)g_chip_erase, 8,
            SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    wait_wip(flash_dev);
    printf("erase done\n");
#endif

    qpi_flash_set_quad_enable(flash_dev);

    // Set read parameters (dummy cycles)
    pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
            SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    pi_spi_send(qspi_dev, (void*)g_set_qspif_dummy, 2*8,
            SPI_LINES_FLAG | PI_SPI_CS_AUTO);
}

static void wait_wip(spi_flash_t *flash_dev)
{
    volatile uint8_t wip = 0;
    uint8_t *status_reg = (uint8_t*)flash_dev->status_reg;
    uint8_t *cmd_buf = (uint8_t*)flash_dev->cmd_buf;
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;
    do
    {
        aos_msleep(5);
        cmd_buf[0] = QSPIF_READ_STATUS_CMD; // read status reg
        pi_spi_send(qspi_dev, (void*)&cmd_buf[0], 1*8,
                SPI_LINES_FLAG | PI_SPI_CS_KEEP);
        pi_spi_receive(qspi_dev, (void*)&status_reg[BUSY_BIT_ID], 1*8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO);
        wip = IS_BUSY(status_reg);
    }while(wip);
}

static inline void pi_qpi_flash_conf_spi(struct pi_spi_conf *conf,
        struct pi_spiflash_conf *flash_conf)
{
    memset(conf, 0, sizeof(struct pi_spi_conf));
    pi_spi_conf_init(conf);
    // we remove 1500 to make sure divisor will be okay even if freq is not 
    // precisely at target. Value is empiric
    // we then mult by two to workaround divisor bogus spec
    conf->max_baudrate = flash_conf->baudrate;
    conf->wordsize = PI_SPI_WORDSIZE_8;
    conf->polarity = 0;
    conf->phase = conf->polarity;
    conf->cs = flash_conf->spi_cs;
    conf->itf = flash_conf->spi_itf;
    conf->big_endian = 1;
}

static int spiflash_open(struct pi_device *bsp_flash_dev)
{
    spi_flash_t *flash_dev = pi_l2_malloc(sizeof(spi_flash_t));
    if(flash_dev == NULL)
    {
        return -1;
    }
    memset(flash_dev, 0, sizeof(spi_flash_t));
    struct pi_spiflash_conf *flash_conf = bsp_flash_dev->config;
    pi_qpi_flash_conf_spi(&flash_dev->qspi_conf, flash_conf);
    pi_open_from_conf(&flash_dev->qspi_dev,&flash_dev->qspi_conf);
    int ret = pi_spi_open(&flash_dev->qspi_dev);
    if(ret)
    {
        printf("spi flash: failed to open spi itf\n");
        return ret;
    }

    flash_dev->sector_size = flash_conf->sector_size;

    qpi_flash_pre_config(flash_dev);
    bsp_flash_dev->data = (void*)flash_dev;
    return 0;
}

void spiflash_close(struct pi_device *bsp_flash_dev)
{
    spi_flash_t *flash_dev = (spi_flash_t*)bsp_flash_dev->data;
    // wait for last operations
    wait_wip(flash_dev);
    pi_spi_close(&flash_dev->qspi_dev);
    pi_l2_free(flash_dev, sizeof(spi_flash_t));
}

int spiflash_erase_chip(struct pi_device *bsp_flash_dev)
{
    spi_flash_t *flash_dev = (spi_flash_t*) bsp_flash_dev->data;

    pi_device_t *qspi_dev = &flash_dev->qspi_dev;
    // flash chip erase (optional!)
    pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
            SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    pi_spi_send(qspi_dev, (void*)g_chip_erase, 8,
            SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    wait_wip(flash_dev);
    return 0;
}

int spiflash_erase(struct pi_device *bsp_flash_dev, uint32_t addr, int size)
{
    spi_flash_t *flash_dev  =(spi_flash_t*) bsp_flash_dev->data;
    uint32_t flash_addr = addr;
    uint32_t sector_size = flash_dev->sector_size;
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;

    uint8_t *cmd_buf = (uint8_t*)flash_dev->ucode_buffer;
    for(uint32_t c_size = 0; c_size < size; c_size += sector_size)
    {
        uint32_t curr_flash_addr = flash_addr+c_size;
        cmd_buf[0] = QSPIF_ERASE_SECTOR_CMD;
        cmd_buf[1] = (curr_flash_addr & 0x00FF0000)>>16;
        cmd_buf[2] = (curr_flash_addr & 0x0000FF00)>>8;
        cmd_buf[3] = (curr_flash_addr & 0x000000FF)>>0;
        pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO);
        pi_spi_send(qspi_dev, (void*)cmd_buf, 8*QSPIF_ERASE_SIZE,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO);
        wait_wip(flash_dev);
    }
    return 0;
}

int spiflash_erase_sector(struct pi_device *bsp_flash_dev, uint32_t addr)
{
    spi_flash_t *flash_dev  = (spi_flash_t*)bsp_flash_dev->data;
    uint32_t flash_addr = addr;
    uint32_t sector_size = flash_dev->sector_size;
    
    return spiflash_erase(bsp_flash_dev, addr, sector_size);
}



static int spiflash_read(struct pi_device *bsp_flash_dev, uint32_t addr, void *data,
        uint32_t size)
{
    spi_flash_t *flash_dev  =(spi_flash_t*) bsp_flash_dev->data;
    uint32_t flash_addr = addr;
    uint32_t sector_size = flash_dev->sector_size;
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;

    uint8_t *l2_buff = pi_l2_malloc(QSPIF_PAGE_SIZE);
    uint8_t *ucode = (uint8_t*)flash_dev->ucode_buffer;
    volatile uint32_t *ucode_u32 = (uint32_t*)ucode;
    if(!l2_buff)
    {
        printf("MALLOC FAILED\n");
        return -1;
    }
    uint32_t size_left = size;
    uint32_t curr_size = 0;
    uint32_t curr_pos = 0;
    while(size_left)
    {
        if(size_left >= QSPIF_PAGE_SIZE)
        {
            curr_size = QSPIF_PAGE_SIZE;
            size_left -= QSPIF_PAGE_SIZE;
        }
        else
        {
            curr_size = size_left;
            size_left = 0;
        }
        uint32_t curr_addr = flash_addr+curr_pos;
        ucode_u32[0] = pi_spi_get_config(qspi_dev);
        ucode_u32[1] = SPI_CMD_SOT(0);
        ucode_u32[2] = SPI_CMD_TX_DATA(8*1, 0, 0);
        ucode[12] = QSPIF_QIO_FAST_READ_CMD;// use QO for gvsoc instead
        ucode_u32[4] = SPI_CMD_TX_DATA(8*3, 1, 0);
        ucode[20] = (curr_addr >> 16 )& 0xFFUL;
        ucode[21] = (curr_addr >> 8) & 0xFFUL;
        ucode[22] = curr_addr & 0xFFUL;
        ucode_u32[6] = SPI_CMD_DUMMY(DUMMY_CYCLES);
        ucode_u32[7] = SPI_CMD_RX_DATA(curr_size*8, 1, 0);// use 4 lines to recv
        ucode_u32[8] = SPI_CMD_EOT(1);
        // any write/erase op must be preceeded by a WRITE ENABLE op, 
        // with full CS cycling
        pi_spi_receive_with_ucode(qspi_dev, (void*)l2_buff, (curr_size)*8,
                PI_SPI_LINES_SINGLE | PI_SPI_CS_AUTO, QSPIF_RCV_UCODE_SIZE,
                ucode);

        memcpy(data+curr_pos, l2_buff, curr_size);
        curr_pos += curr_size;
    }
    pi_l2_free(l2_buff, QSPIF_PAGE_SIZE);
    return 0;

}
    
static int spiflash_program(struct pi_device *bsp_flash_dev, uint32_t flash_addr,
      const void *data, uint32_t size)
{
    spi_flash_t *flash_dev  = (spi_flash_t*) bsp_flash_dev->data;
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;

    uint8_t *l2_buff = pi_l2_malloc(QSPIF_PAGE_SIZE);
    if(!l2_buff)
    {
        printf("Malloc failed!\n");
        return -1;
    }
    uint32_t size_left = size;
    uint32_t curr_size = 0;
    uint32_t curr_pos = 0;
    
    uint8_t *ucode = (uint8_t*)flash_dev->ucode_buffer;
    uint32_t *ucode_u32 = (uint32_t*)ucode;
    if((flash_addr & 0xFF) && (((flash_addr & 0xFF)+size_left) > 0x100))
    {
        curr_pos  = 0;
        curr_size = 0x100 - (flash_addr & 0xFF);
        size_left -= curr_size;
        // any write/erase op must be preceeded by a WRITE ENABLE op,
        // with full CS cycling
        pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO);
        memcpy(l2_buff, data+curr_pos, curr_size);
        ucode_u32[0] = pi_spi_get_config(qspi_dev);
        ucode_u32[1] = SPI_CMD_SOT(0);
        ucode_u32[2] = SPI_CMD_TX_DATA(8*4, 0, 0);
        ucode[12] = QSPIF_QIO_PAGE_PROG_CMD;//0x38;
        ucode[13] = ((flash_addr+curr_pos) & 0x00FF0000)>>16;
        ucode[14] = ((flash_addr+curr_pos) & 0x0000FF00)>>8;
        ucode[15] = ((flash_addr+curr_pos) & 0x000000FF)>>0;
        ucode_u32[4] = SPI_CMD_TX_DATA(curr_size*8, 1, 0);
        pi_spi_send_with_ucode(qspi_dev, (void*)l2_buff, (curr_size)*8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO, QSPIF_PPQ_UCODE_SIZE, ucode);
        wait_wip(flash_dev);
        curr_pos += curr_size;
    }

    while(size_left)
    {
        if(size_left >= QSPIF_PAGE_SIZE)
        {
            curr_size = QSPIF_PAGE_SIZE;
            size_left -= QSPIF_PAGE_SIZE;
        }
        else
        {
            curr_size = size_left;
            size_left = 0;
        }
        memcpy(l2_buff, data+curr_pos, curr_size);
        // any write/erase op must be preceeded by a WRITE ENABLE op,
        // with full CS cycling
        pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO);
        ucode_u32[0] = pi_spi_get_config(qspi_dev);
        ucode_u32[1] = SPI_CMD_SOT(0);
        ucode_u32[2] = SPI_CMD_TX_DATA(8*4, 0, 0);
        ucode[12] = QSPIF_QIO_PAGE_PROG_CMD;
        ucode[13] = ((flash_addr+curr_pos) & 0x00FF0000)>>16;
        ucode[14] = ((flash_addr+curr_pos) & 0x0000FF00)>>8;
        ucode[15] = ((flash_addr+curr_pos) & 0x000000FF)>>0;
        ucode_u32[4] = SPI_CMD_TX_DATA(curr_size*8, 1, 0);
        pi_spi_send_with_ucode(qspi_dev, (void*)l2_buff, (curr_size)*8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO, QSPIF_PPQ_UCODE_SIZE, ucode);
        wait_wip(flash_dev);
        curr_pos += curr_size;
    }
    pi_l2_free(l2_buff, QSPIF_PAGE_SIZE);
    pi_l2_free(ucode, QSPIF_PPQ_UCODE_SIZE);
    return 0;
}

int spiflash_reg_set(struct pi_device *bsp_flash_dev, uint32_t reg, uint8_t *value)
{
    spi_flash_t *flash_dev  = (spi_flash_t*)bsp_flash_dev->data;
    uint32_t sector_size = flash_dev->sector_size;
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;

    uint8_t *ucode = (uint8_t*)flash_dev->ucode_buffer;
    uint32_t *ucode_u32 = (uint32_t*)ucode;

    pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    ucode[0] = (uint8_t) reg;
    ucode[1] = *value;
    pi_spi_send(qspi_dev, (void*)ucode, 16, SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    return 0;
}

int spiflash_reg_get(struct pi_device *bsp_flash_dev, uint32_t reg, uint8_t *value)
{
    spi_flash_t *flash_dev  =(spi_flash_t*) bsp_flash_dev->data;
    uint32_t sector_size = flash_dev->sector_size;
    pi_device_t *qspi_dev = &flash_dev->qspi_dev;

    uint8_t *ucode = (uint8_t*)flash_dev->ucode_buffer;
    uint32_t *ucode_u32 = (uint32_t*)ucode;

    pi_spi_send(qspi_dev, (void*)g_write_enable, 8,
                SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    ucode[0] = (uint8_t) reg;
    pi_spi_send(qspi_dev, (void*)ucode, 8, SPI_LINES_FLAG | PI_SPI_CS_KEEP);
    pi_spi_receive(qspi_dev, (void*)value, 8, SPI_LINES_FLAG | PI_SPI_CS_AUTO);
    return 0;
}

int spiflash_copy(struct pi_device *device, uint32_t flash_addr, void *buffer,
        uint32_t size, int ext2loc)
{
    int ret = 0;
    if(!ext2loc)
    {
        ret = spiflash_program(device, flash_addr, buffer, size);
    }
    else
    {
        ret = spiflash_read(device, flash_addr, buffer, size);
    }
    return ret;
}

int spiflash_copy_2d(struct pi_device *device, uint32_t flash_addr,
        void *buffer, uint32_t size, uint32_t stride, uint32_t length,
        int ext2loc)
{
    // not yet implemented
    return -1;
}

static int32_t spiflash_ioctl(struct pi_device *device, uint32_t cmd, void *arg)
{
    return 0;
}

static void spiflash_read_async(struct pi_device *device, uint32_t addr, void *data,
        uint32_t size, pi_task_t *task)
{
    spiflash_read(device,addr,data,size);
    pi_task_release(task);
}

static void spiflash_program_async(struct pi_device *device, uint32_t addr,
        const void *data, uint32_t size, pi_task_t *task)
{
    spiflash_program(device,addr,data,size);
    pi_task_release(task);
}

static void spiflash_erase_chip_async(pi_device_t *device, pi_task_t *task)
{
    spiflash_erase_chip(device);
    pi_task_release(task);
}

static void spiflash_erase_sector_async(struct pi_device *device, uint32_t addr,
        pi_task_t *task)
{
    spiflash_erase_sector(device,addr);
    pi_task_release(task);
}

static void spiflash_erase_async(struct pi_device *device, uint32_t addr,
    int size, pi_task_t *task)
{
    spiflash_erase(device,addr,size);
    pi_task_release(task);
}

static void spiflash_reg_set_async(struct pi_device *device, uint32_t addr,
        uint8_t *value, pi_task_t *task)
{
    spiflash_reg_set(device,addr,value);
    pi_task_release(task);
}

static void spiflash_reg_get_async(struct pi_device *device, uint32_t addr,
        uint8_t *value, pi_task_t *task)
{
    spiflash_reg_set(device,addr,value);
    pi_task_release(task);
}

static int spiflash_copy_async(struct pi_device *device, uint32_t flash_addr,
        void *buffer, uint32_t size, int ext2loc, pi_task_t *task)
{
    int ret = spiflash_copy(device, flash_addr, buffer, size, ext2loc);
    pi_task_release(task);
    return ret;
}

static int spiflash_copy_2d_async(struct pi_device *device, uint32_t flash_addr,
        void *buffer, uint32_t size, uint32_t stride, uint32_t length,
        int ext2loc, pi_task_t *task)
{
    int ret = spiflash_copy_2d(device, flash_addr, buffer, size, stride, length,
            ext2loc);
    pi_task_release(task);
    return ret;
}

static pi_flash_api_t spiflash_api = {
  .open                 = &spiflash_open,
  .close                = &spiflash_close,
  .ioctl                = &spiflash_ioctl,
  .read_async           = &spiflash_read_async,
  .program_async        = &spiflash_program_async,
  .erase_chip_async     = &spiflash_erase_chip_async,
  .erase_sector_async   = &spiflash_erase_sector_async,
  .erase_async          = &spiflash_erase_async,
  .reg_set_async        = &spiflash_reg_set_async,
  .reg_get_async        = &spiflash_reg_get_async,
  .copy_async           = &spiflash_copy_async,
  .copy_2d_async        = &spiflash_copy_2d_async,
  .read                 = &spiflash_read,
  .program              = &spiflash_program,
  .erase_chip           = &spiflash_erase_chip,
  .erase_sector         = &spiflash_erase_sector,
  .erase                = &spiflash_erase,
  .reg_set              = &spiflash_reg_set,
  .reg_get              = &spiflash_reg_get,
  .copy                 = &spiflash_copy,
  .copy_2d              = &spiflash_copy_2d,
};

void pi_spiflash_conf_init(struct pi_spiflash_conf *conf)
{
    conf->flash.api = &spiflash_api;
    bsp_spiflash_conf_init(conf);
    __flash_conf_init(&conf->flash);
}
