/* 
 * Copyright (C) 2019 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 * Authors: Germain Haugou, ETH (germain.haugou@iis.ee.ethz.ch)
 */

#include "bsp/bsp.h"

#if defined(CONFIG_HIMAX)
#define WIDTH    324
#define HEIGHT   244
#else
#define WIDTH    320
#define HEIGHT   240
#endif

static inline int get_nb_buffers()
{
  if (rt_platform() == ARCHI_PLATFORM_RTL)
    return 4;
  else
    return 4;
}

#define BUFF_SIZE (WIDTH*HEIGHT)

RT_L2_DATA unsigned char *buff[16];

int main()
{
  printf("Entering main controller\n");

  struct pi_device device;

#if defined(CONFIG_HIMAX)
  printf("Opening Himax camera\n");
  struct himax_conf cam_conf;
  himax_conf_init(&cam_conf);
#elif defined(CONFIG_MT9V034)
  printf("Opening mt9v034 camera\n");
  struct mt9v034_conf cam_conf;
  mt9v034_conf_init(&cam_conf);
#endif

  cam_conf.format = CAMERA_QVGA;

  pi_open_from_conf(&device, &cam_conf);
  if (camera_open(&device))
    return -1;

  for (int i=0; i<get_nb_buffers(); i++)
  {
    buff[i] = pmsis_l2_malloc(WIDTH*HEIGHT);
    if (buff[i] == NULL) {
      printf("Unable to allocate buffer\n");
      goto error;
    }
  }

  for (int i=0; i<get_nb_buffers(); i++)
  {
    pi_task_t task;

    camera_capture_async(&device, buff[i], WIDTH*HEIGHT, pi_task(&task));
    camera_control(&device, CAMERA_CMD_START, 0);

    pi_wait_on_task(&task);
    camera_control(&device, CAMERA_CMD_STOP, 0);
    

    // Now wait some time to start capturing in the middle of the next frame
    rt_time_wait_us((i + 1) * 5000);
  }


  for (int j=0; j<get_nb_buffers(); j++)
  {
    for (int i=0; i<BUFF_SIZE; i++)
    {
      unsigned char expected;
      expected = i & 0x7f;
      if (expected != buff[j][i])
      {
        printf("Error, buffer: %d, index: %d, expected: 0x%x, read: 0x%x\n", j, i, expected, buff[j][i]);
        goto error;
      }
    }
  }

  camera_close(&device);
  printf("Test success\n");


  return 0;

error:
  printf("Test failure\n");
  return -1;
}
