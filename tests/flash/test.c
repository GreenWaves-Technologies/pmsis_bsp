/* 
 * Copyright (C) 2019 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Germain Haugou, GreenWaves Technologies (germain.haugou@greenwaves-technologies.com)
 */

#include <bsp/bsp.h>
#include "stdio.h"

#define HYPER_FLASH 0
#define SPI_FLASH   1
#define MRAM        2

#define BUFF_SIZE 2048
#define PROGRAM_SIZE_RTL   BUFF_SIZE
#define PROGRAM_SIZE_OTHER ((1<<18)*4)

#define NB_ITER 2


static inline void get_info(unsigned int *program_size)
{
  if (rt_platform() == ARCHI_PLATFORM_RTL)
  {
    *program_size = PROGRAM_SIZE_RTL;
  }
  else
  {
    *program_size = PROGRAM_SIZE_OTHER;
  }
}


static L2_DATA unsigned char rx_buffer[BUFF_SIZE];
static L2_DATA unsigned char tx_buffer[BUFF_SIZE];

int main()
{
  struct pi_device flash;
  struct hyperflash_conf flash_conf;
  struct flash_info flash_info;

  printf("Entering main controller\n");

  hyperflash_conf_init(&flash_conf);

#if FLASH_TYPE == HYPER_FLASH
  flash_conf.type = FLASH_TYPE_HYPER;
#elif FLASH_TYPE == SPI_FLASH
  flash_conf.type = FLASH_TYPE_SPI;
#elif FLASH_TYPE == MRAM
  flash_conf.type = FLASH_TYPE_MRAM;
#else
  #error Unknown flash
#endif

  pi_open_from_conf(&flash, &flash_conf);

  if (flash_open(&flash))
    return -1;

  flash_ioctl(&flash, FLASH_IOCTL_INFO, (void *)&flash_info);

  for (int j=0; j<NB_ITER; j++)
  {
    // The beginning of the flash may contain runtime data such as the boot binary.
    // Round-up the flash start with the sector size to not erase it.
    // Also add small offset to better test erase (sector size aligned) and program (512 byte aligned).
    uint32_t flash_addr = ((flash_info.flash_start + flash_info.sector_size - 1) & ~(flash_info.sector_size - 1)) + 128;

    unsigned int size;
    get_info(&size);

    flash_erase(&flash, flash_addr, size);

    while(size > 0)
    {
      for (int i=0; i<BUFF_SIZE; i++)
      {
        tx_buffer[i] = i*(j+1);
        rx_buffer[i] = 0;
      }

      flash_program(&flash, flash_addr, tx_buffer, BUFF_SIZE);
      flash_read(&flash, flash_addr, rx_buffer, BUFF_SIZE);

      for (int i=0; i<BUFF_SIZE; i++)
      {
          if (rx_buffer[i] != (unsigned char)(i*(j+1)))
          {
            printf("Error at index %d, expected 0x%2.2x, got 0x%2.2x\n", i, (unsigned char)i, rx_buffer[i]);
            printf("TEST FAILURE\n");
            return -1;
          }
      }
      size -= BUFF_SIZE;
      flash_addr += BUFF_SIZE;
    }
  }

  flash_close(&flash);

  printf("TEST SUCCESS\n");

  return 0;
}
