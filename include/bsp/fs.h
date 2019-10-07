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

#include "pmsis.h"

/**
 * @defgroup FS File-System
 *
 * The file-system driver provides support for accessing files on a flash. The
 * following file-systems are available:
 *  - Read-only file system. This file-system is very basic but quite-convenient
 *    to have access to input data. The open operation does not scale well when
 *    having lots of file so this file-system should only be used with few
 *    files.
 *
 */

/**
 * @addtogroup FS
 * @{
 */

/**@{*/



/** \enum fs_type_e
 * \brief File-system type.
 *
 * This can be used to select the type of file-system to mount.
 */
typedef enum {
  FS_READ_ONLY     = 0     /*!< Read-only file system. */
} fs_type_e;



/** \struct fs_conf
 * \brief File-system configuration structure.
 *
 * This structure is used to pass the desired file-system configuration to the
 * runtime when mounting the file-system.
 */
struct fs_conf {
  fs_type_e type;           /*!< File-system type. */
  struct pi_device *flash;  /*!< Flash device. The flash device must be first
    opened and its device structure passed here. */
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
  unsigned char *cache;
  unsigned int  cache_addr;
} fs_file_t;

typedef struct {
  unsigned int addr;
  unsigned int size;
  unsigned int path_size;
  char name[];
} fs_desc_t;

typedef struct cl_fs_req_s cl_fs_req_t;

/** \brief Initialize a file-system configuration with default values.
 *
 * The structure containing the configuration must be kept allocated until
 * the file-system is mounted.
 *
 * \param conf A pointer to the file-system configuration.
 */
void fs_conf_init(struct fs_conf *conf);

/** \brief Mount a file-system.
 *
 * This function must be called before the file-system device can be used.
 * It will do all the needed configuration to make it usable and initialize
 * the handle used to refer to this opened device when calling other functions.
 *
 * \param device    A pointer to the device structure of the device to open.
 *   This structure is allocated by the called and must be kept alive until the
 *   device is closed.
 * \return          0 if the operation is successfull, -1 if there was an error.
 */
int32_t fs_mount(struct pi_device *device);

/** \brief Unmount a mounted file-system.
 *
 * This function can be called to close a mounted file-system once it is not needed anymore, in order to free
 * all allocated resources. Once this function is called, the file-system is not accessible anymore and must be mounted
 * again before being used.
 * This operation is asynchronous and its termination can be managed through an event.
 * This can only be called on the fabric-controller.
 *
 * \param handle    The handle of the file-system which was returned when the file-system was mounted.
 * \param event     The event used for managing termination.
 */
void fs_unmount(struct pi_device *device);

/** \brief Open a file.
 *
 * This function can be called to open a file on a file-system in order to read or write data to it.
 * This operation is asynchronous and its termination can be managed through an event.
 * This can only be called on the fabric-controller.
 *
 * \param fs        The handle of the file-system which was returned when the file-system was mounted.
 * \param file      The path to the file to be opened.
 * \param flags     Optional flags to configure how the file is opened.
 * \param event     The event used for managing termination.
 * \return          NULL if the file is not found, or a handle identifying the file which can be used with other functions.
 */
fs_file_t *fs_open(struct pi_device *device, const char *file, int flags);

/** \brief Close a file.
 *
 * This function can be called to close an opened file once it is not needed anymore in order to free the allocated resources.
 * This operation is asynchronous and its termination can be managed through an event.
 * This can only be called on the fabric-controller.
 *
 * \param file      The handle of the file to be closed.
 * \param event     The event used for managing termination.
 */
void fs_close(fs_file_t *file);

/** \brief Read data from a file.
 *
 * This function can be called to read data from an opened file. The data is read from the current position which
 * is the beginning of the file when the file is opened. The current position is incremented by the number of
 * bytes read by the call to this function.
 * This operation is asynchronous and its termination can be managed through an event.
 * This can only be called on the fabric-controller.
 * Compared to rt_fs_direct_read, this functions can use intermediate transfers to support any alignment constraint
 * from the flash. So it can be slower in case part of the transfer has to be emulated.
 *
 * \param file      The handle of the file where to read data.
 * \param buffer    The memory location where the read data must be copied.
 * \param size      The size in bytes to read from the file.
 * \param event     The event used for managing termination.
 * \return          The number of bytes actually read from the file. This can be smaller than the requested size if the end of file is reached.
 */
int fs_read(fs_file_t *file, void *buffer, size_t size);

int fs_read_async(fs_file_t *file, void *buffer, size_t size, pi_task_t *task);

/** \brief Read data from a file with no intermediate cache.
 *
 * This function can be called to read data from an opened file. The data is read from the current position which
 * is the beginning of the file when the file is opened. The current position is incremented by the number of
 * bytes read by the call to this function.
 * This operation is asynchronous and its termination can be managed through an event.
 * This can only be called on the fabric-controller.
 * Compared to rt_fs_read, this function does direct read transfers from the flash. So the flash address and the size
 * of the transfer can have some constraints depending on the flash.
 *
 * \param file      The handle of the file where to read data.
 * \param buffer    The memory location where the read data must be copied.
 * \param size      The size in bytes to read from the file.
 * \param event     The event used for managing termination.
 * \return          The number of bytes actually read from the file. This can be smaller than the requested size if the end of file is reached.
 */
int fs_direct_read(fs_file_t *file, void *buffer, size_t size);

int fs_direct_read_async(fs_file_t *file, void *buffer, size_t size, pi_task_t *task);

/** \brief Reposition the current file position.
 *
 * This function can be called to change the current position of a file.
 * Note that this does not affect pending copies, but only the ones which will be enqueued after this call.
 * This can only be called on the fabric-controller.
 *
 * \param file      The handle of the file for which the current position is changed.
 * \param offset    The offset where to set the current position. The offset can be between 0 for the beginning of the file and the file size.
 * \return          RT_STATUS_OK if the operation was successful, RT_STATUS_ERR otherwise.
 */
int fs_seek(fs_file_t *file, unsigned int offset);

int fs_copy(fs_file_t *file, uint32_t index, void *buffer, uint32_t size, int ext2loc);

int fs_copy_2d(fs_file_t *file, uint32_t index, void *buffer, uint32_t size, uint32_t stride, uint32_t length, int ext2loc);

int fs_copy_async(fs_file_t *file, uint32_t index, void *buffer, uint32_t size, int ext2loc, pi_task_t *task);

int fs_copy_2d_async(fs_file_t *file, uint32_t index, void *buffer, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, pi_task_t *task);

/** \brief Read data from a file from cluster side.
 *
 * This function implements the same feature as rt_fs_read but can be called from cluster side in order to expose
 * the feature on the cluster.
 * This function can be called to read data from an opened file. The data is read from the current position which
 * is the beginning of the file when the file is opened. The current position is incremented by the number of
 * bytes read by the call to this function.
 * This operation is asynchronous and its termination is managed through the request structure.
 * This can only be called on the cluster.
 * Compared to rt_fs_direct_read, this functions can use intermediate transfers to support any alignment constraint
 * from the flash. So it can be slower in case part of the transfer has to be emulated.
 * The only difference compared to rt_fs_read is that the file position is automatically set to 0 for the next
 * transfer if the current transfer reaches the end of the file.
 *
 * \param file      The handle of the file where to read data.
 * \param buffer    The memory location where the read data must be copied.
 * \param size      The size in bytes to read from the file.
 * \param req       The request structure used for termination.
 */
void cl_fs_read(fs_file_t *file, void *buffer, size_t size, cl_fs_req_t *req);

/** \brief Read data from a file with no intermediate cache from cluster side.
 *
 * This function implements the same feature as rt_fs_direct_read but can be called from cluster side in order to expose
 * the feature on the cluster.
 * This function can be called to read data from an opened file. The data is read from the current position which
 * is the beginning of the file when the file is opened. The current position is incremented by the number of
 * bytes read by the call to this function.
 * This operation is asynchronous and its termination can be managed through an event.
 * Can only be called from cluster side.
 * Compared to rt_fs_direct_read, this function does direct read transfers from the flash. So the flash address and the size
 * of the transfer can have some constraints depending on the flash.
 *
 * \param file      The handle of the file where to read data.
 * \param buffer    The memory location where the read data must be copied.
 * \param size      The size in bytes to read from the file.
 * \param req       The request structure used for termination.
 */
void cl_fs_direct_read(fs_file_t *file, void *buffer, size_t size, cl_fs_req_t *req);

/** \brief Reposition the current file position from cluster side.
 *
 * This function can be called from cluster side to change the current position of a file.
 * Note that this does not affect pending copies, but only the ones which will be enqueued after this call.
 *
 * \param file      The handle of the file for which the current position is changed.
 * \param offset    The offset where to set the current position. The offset can be between 0 for the beginning of the file and the file size.
 * \param req       The request structure used for termination.
 */
void cl_fs_seek(fs_file_t *file, unsigned int offset, cl_fs_req_t *req);

void cl_fs_copy(fs_file_t *file, uint32_t index, void *buffer, uint32_t size, int ext2loc, cl_fs_req_t *req);

void cl_fs_copy_2d(fs_file_t *file, uint32_t index, void *buffer, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, cl_fs_req_t *req);

/** \brief Wait until the specified fs request has finished.
 *
 * This blocks the calling core until the specified cluster remote copy is finished.
 * As the remote copy is asynchronous, this also gives the number of bytes which was read.
 * Can only be called from cluster side.
 *
 * \param req       The request structure used for termination.
 * \return          The number of bytes actually read from the file. This can be smaller than the requested size if the end of file is reached.
 *                  Could be also RT_STATUS_OK if the rt_fs_cluster_seek was successful, RT_STATUS_ERR otherwise.
 */
static inline int cl_fs_wait(cl_fs_req_t *req);


/// @cond IMPLEM

typedef enum {
  FS_MOUNT_FLASH_ERROR     = 1,     /*!< There was an error mounting the flash filesystem. */
  FS_MOUNT_MEM_ERROR       = 2      /*!< There was an error allocating memory when mounting the file-system. */
} fs_error_e;

typedef struct fs_l2_s
{
  uint32_t fs_offset;
  uint32_t reserved0;
  uint32_t fs_size;
  uint32_t reserved1;
} fs_l2_t;

typedef struct fs_s
{
  struct pi_device *flash;
  pi_task_t step_event;
  pi_task_t *pending_event;
  int mount_step;
  int fs_size;
  fs_l2_t *fs_l2;
  unsigned int *fs_info;
  int nb_comps;
  //rt_mutex_t mutex;
  pi_task_t event;
  int error;
} fs_t;


typedef struct cl_fs_req_s
{
  fs_file_t *file;
  uint32_t index;
  void *buffer;
  size_t size;
  uint32_t stride;
  uint32_t length;
  uint32_t ext2loc;
  pi_task_t task;
  uint8_t done;
  int result;
  unsigned char cid;
  unsigned char direct;
  unsigned int offset;
} cl_fs_req_t;

static inline __attribute__((always_inline)) int cl_fs_wait(cl_fs_req_t *req)
{
    #if defined(PMSIS_DRIVERS)
    cl_wait_task(&(req->done));
    #else
    while((*(volatile char *)&req->done) == 0)
    {
        eu_evt_maskWaitAndClr(1<<RT_CLUSTER_CALL_EVT);
    }
    #endif  /* PMSIS_DRIVERS */
    return req->result;
}

/// @endcond

#endif
