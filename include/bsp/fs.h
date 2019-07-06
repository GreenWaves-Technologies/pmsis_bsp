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

#ifndef __FS__FS__H__
#define __FS__FS__H__



typedef enum {
  FS_MOUNT_FLASH_ERROR     = 1,     /*!< There was an error mounting the flash filesystem. */
  FS_MOUNT_MEM_ERROR       = 2      /*!< There was an error allocating memory when mounting the file-system. */
} fs_error_e;



typedef enum {
  FS_READ_ONLY     = 0     /*!< Read-only file system. */
} fs_type_e;



struct fs_conf {
  fs_type_e type;     /*!< File-system type. */
  struct pi_device *flash;        /*!< Flash configuration. */
};

typedef struct fs_file_s fs_file_t;

typedef struct fs_file_s {
  unsigned int offset;
  unsigned int size;
  unsigned int addr;
  unsigned int pending_addr;
  struct pi_device *fs;
  pi_task_t *pending_event;
  pi_task_t step_event;
  unsigned int pending_buffer;
  unsigned int pending_size;
} fs_file_t;

typedef struct {
  unsigned int addr;
  unsigned int size;
  unsigned int path_size;
  char name[];
} fs_desc_t;


void fs_conf_init(struct fs_conf *conf);



int fs_mount(struct pi_device *device);



void fs_unmount(struct pi_device *device);



fs_file_t *fs_open(struct pi_device *device, const char *file, int flags);



void fs_close(fs_file_t *file);



int fs_read(fs_file_t *file, void *buffer, size_t size);



int fs_read_async(fs_file_t *file, void *buffer, size_t size, pi_task_t *task);




int fs_direct_read(fs_file_t *file, void *buffer, size_t size);

int fs_direct_read_async(fs_file_t *file, void *buffer, size_t size, pi_task_t *task);



int fs_seek(fs_file_t *file, unsigned int offset);


typedef struct fs_l2_s {
  uint32_t fs_offset;
  uint32_t reserved0;
  uint32_t fs_size;
  uint32_t reserved1;
} fs_l2_t;

typedef struct fs_s {
  struct pi_device *flash;
  pi_task_t step_event;
  pi_task_t *pending_event;
  int mount_step;
  int fs_size;
  fs_l2_t *fs_l2;
  unsigned int *fs_info;
  int nb_comps;
  unsigned char *cache;
  unsigned int  cache_addr;
  //rt_mutex_t mutex;
  pi_task_t event;
  int error;
} fs_t;


typedef struct {
  fs_file_t *file;
  void *buffer;
  size_t size;
  pi_task_t task;
  int done;
  int result;
  unsigned char cid;
  unsigned char direct;
  unsigned int offset;
} cl_fs_req_t;


static inline void cl_fs_read(fs_file_t *file, void *buffer, size_t size, cl_fs_read_req_t *req);



static inline void fs_cluster_direct_read(fs_file_t *file, void *buffer, size_t size, cl_fs_req_t *req);



static inline void fs_cluster_seek(fs_file_t *file, unsigned int offset, cl_fs_req_t *req);


static inline int fs_cluster_wait(cl_fs_req_t *req);

#endif
