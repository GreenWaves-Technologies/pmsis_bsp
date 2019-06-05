#include "bsp/display/ili9341.h"
#include "bsp/camera/himax.h"
#include "bsp/camera/mt9v034.h"

#define QVGA 1
//#define QQVGA 1

#ifdef QVGA
#ifdef HIMAX
#define CAM_WIDTH    324
#define CAM_HEIGHT   244
#else
#define CAM_WIDTH    320
#define CAM_HEIGHT   240
#endif
#else
#define CAM_WIDTH    160
#define CAM_HEIGHT   120
#endif

#define LCD_WIDTH    320
#define LCD_HEIGHT   240

static pi_task_t task;
static unsigned char *imgBuff0;
static struct pi_device ili;
static pi_buffer_t buffer;
static struct pi_device device;
static uint64_t fb;


static void lcd_handler(void *arg);
static void cam_handler(void *arg);


static void cam_handler(void *arg)
{
  camera_control(&device, CAMERA_CMD_STOP, 0);

#ifdef USE_BRIDGE
  rt_bridge_fb_update(fb, (unsigned int)imgBuff0, 0, 0, CAM_WIDTH, CAM_HEIGHT, NULL);
  camera_capture_async(&device, imgBuff0, CAM_WIDTH*CAM_HEIGHT, pi_task_callback(&task, cam_handler, NULL));
  camera_control(&device, CAMERA_CMD_START, 0);

#else
  display_write_async(&ili, &buffer, 0, 0, LCD_WIDTH, LCD_HEIGHT, pi_task_callback(&task, lcd_handler, NULL));
#endif
}


static void lcd_handler(void *arg)
{
  camera_control(&device, CAMERA_CMD_START, 0);
  camera_capture_async(&device, imgBuff0, CAM_WIDTH*CAM_HEIGHT, pi_task_callback(&task, cam_handler, NULL));

}


static int open_bridge()
{
  rt_bridge_connect(1, NULL);

  fb = rt_bridge_fb_open("Camera", CAM_WIDTH, CAM_HEIGHT, RT_FB_FORMAT_GRAY, NULL);
  if (fb == 0) return -1;

  return 0;
}


static int open_display(struct pi_device *device)
{
#ifndef USE_BRIDGE
  struct ili9341_conf ili_conf;

  ili9341_conf_init(&ili_conf);

  pi_open_from_conf(device, &ili_conf);

  if (display_open(device))
    return -1;

#else
  if (open_bridge())
  {
    printf("Failed to open bridge\n");
    return -1;
  }
#endif

  return 0;
}


static int open_camera_himax(struct pi_device *device)
{
  struct himax_conf cam_conf;

  himax_conf_init(&cam_conf);

#ifdef QVGA
  cam_conf.format = CAMERA_QVGA;
#endif

  pi_open_from_conf(device, &cam_conf);
  if (camera_open(device))
    return -1;

  return 0;
}

static int open_camera_mt9v034(struct pi_device *device)
{
  struct mt9v034_conf cam_conf;

  mt9v034_conf_init(&cam_conf);

#ifdef QVGA
  cam_conf.format = CAMERA_QVGA;
#endif
#ifdef QQVGA
  cam_conf.format = CAMERA_QQVGA;
#endif

  pi_open_from_conf(device, &cam_conf);
  if (camera_open(device))
    return -1;
  
  return 0;
}

static int open_camera(struct pi_device *device)
{
#ifdef GAPOC_A
  return open_camera_mt9v034(device);
#endif
#ifdef GAPUINO
  return open_camera_himax(device);
#endif
  return -1;
}

int main()
{
  printf("Entering main controller...\n");

  rt_freq_set(__RT_FREQ_DOMAIN_FC, 250000000);
  
  imgBuff0 = (unsigned char *)rt_alloc( RT_ALLOC_L2_CL_DATA, (CAM_WIDTH*CAM_HEIGHT)*sizeof(unsigned char));
  if (imgBuff0 == NULL) {
      printf("Failed to allocate Memory for Image \n");
      return 1;
  }

  if (open_display(&ili))
  {
    printf("Failed to open display\n");
    return -1;
  }

  if (open_camera(&device))
  {
    printf("Failed to open camera\n");
    return -1;
  }

  buffer.data = imgBuff0+CAM_WIDTH*2+2;
  buffer.stride = 4;

#ifdef HIMAX
  // WIth Himax, propertly configure the buffer to skip boarder pixels
  pi_buffer_init(&buffer, PI_BUFFER_TYPE_L2, imgBuff0+CAM_WIDTH*2+2);
  pi_buffer_set_stride(&buffer, 4);
#else
  pi_buffer_init(&buffer, PI_BUFFER_TYPE_L2, imgBuff0+CAM_WIDTH*2);
#endif
  pi_buffer_set_format(&buffer, PI_BUFFER_FORMAT_GRAY);

  camera_control(&device, CAMERA_CMD_STOP, 0);
  camera_capture_async(&device, imgBuff0, CAM_WIDTH*CAM_HEIGHT, pi_task_callback(&task, cam_handler, &device));
  camera_control(&device, CAMERA_CMD_START, 0);


  while(1)
  {
    pi_yield();
  }

  return 0;
}
