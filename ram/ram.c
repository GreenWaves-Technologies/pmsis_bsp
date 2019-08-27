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

#include "bsp/ram.h"

#ifndef PMSIS_DRIVERS

int ram_open(struct pi_device *device)
{
    struct ram_conf *conf = (struct ram_conf *)device->config;
    ram_api_t *api = (ram_api_t *)conf->api;
    device->api = (struct pi_device_api *)api;
    return api->open(device);
}


void __ram_conf_init(struct ram_conf *conf)
{
}


static void __ram_cluster_req_done(void *_req)
{
    cl_ram_req_t *req = (cl_ram_req_t *)_req;
    #if defined(PMSIS_DRIVERS)
    cl_notify_task_done(&(req->done), req->cid);
    #else
    req->done = 1;
    __rt_cluster_notif_req_done(req->cid);
    #endif  /* PMSIS_DRIVERS */
}

static void __ram_cluster_req(void *_req)
{
    cl_ram_req_t *req = (cl_ram_req_t* )_req;

    if (req->is_2d)
  	ram_copy_2d_async(req->device, req->hyper_addr, req->addr, req->size, req->stride, req->length, req->ext2loc, pi_task_callback(&req->event, __ram_cluster_req_done, (void *)req));
    else
  	ram_copy_async(req->device, req->hyper_addr, req->addr, req->size, req->ext2loc, pi_task_callback(&req->event, __ram_cluster_req_done, (void *)req));
}


void cl_ram_copy(struct pi_device *device,
                 uint32_t hyper_addr, void *addr, uint32_t size, int ext2loc, cl_ram_req_t *req)
{
    req->device = device;
    req->addr = addr;
    req->hyper_addr = hyper_addr;
    req->size = size;
    req->cid = pi_cluster_id();
    req->done = 0;
    req->ext2loc = ext2loc;
    req->is_2d = 0;
    #if defined(__PULP_OS__)
    __rt_task_init_from_cluster(&req->event);
    #endif  /* __PULP_OS__ */
    pi_task_callback(&req->event, __ram_cluster_req, (void* )req);
    #if defined(PMSIS_DRIVERS)
    cl_send_task_to_fc(&(req->event));
    #else
    __rt_cluster_push_fc_event(&req->event);
    #endif  /* PMSIS_DRIVERS */
}



void cl_ram_copy_2d(struct pi_device *device,
                    uint32_t hyper_addr, void *addr, uint32_t size, uint32_t stride, uint32_t length, int ext2loc, cl_ram_req_t *req)
{
    req->device = device;
    req->addr = addr;
    req->hyper_addr = hyper_addr;
    req->size = size;
    req->stride = stride;
    req->length = length;
    req->cid = pi_cluster_id();
    req->done = 0;
    req->ext2loc = ext2loc;
    req->is_2d = 1;
    #if defined(__PULP_OS__)
    __rt_task_init_from_cluster(&req->event);
    #endif  /* __PULP_OS__ */
    pi_task_callback(&req->event, __ram_cluster_req, (void* )req);
    #if defined(PMSIS_DRIVERS)
    cl_send_task_to_fc(&(req->event));
    #else
    __rt_cluster_push_fc_event(&req->event);
    #endif  /* PMSIS_DRIVERS */
}


void __ram_alloc_cluster_req(void *_req)
{
    cl_ram_alloc_req_t *req = (cl_ram_alloc_req_t *)_req;
    req->error = ram_alloc(req->device, &req->result, req->size);
    #if defined(PMSIS_DRIVERS)
    cl_notify_task_done(&(req->done), req->cid);
    #else
    req->done = 1;
    __rt_cluster_notif_req_done(req->cid);
    #endif  /* PMSIS_DRIVERS */
}



void __ram_free_cluster_req(void *_req)
{
    cl_ram_free_req_t *req = (cl_ram_free_req_t *)_req;
    ram_free(req->device, req->chunk, req->size);
    #if defined(PMSIS_DRIVERS)
    cl_notify_task_done(&(req->done), req->cid);
    #else
    req->done = 1;
    __rt_cluster_notif_req_done(req->cid);
    #endif  /* PMSIS_DRIVERS */
}


void cl_ram_alloc(struct pi_device *device, uint32_t size, cl_ram_alloc_req_t *req)
{
    req->device = device;
    req->size = size;
    req->cid = pi_cluster_id();
    req->done = 0;
    #if defined(__PULP_OS__)
    __rt_task_init_from_cluster(&req->event);
    #endif  /* __PULP_OS__ */
    pi_task_callback(&req->event, __ram_alloc_cluster_req, (void* )req);
    #if defined(PMSIS_DRIVERS)
    cl_send_task_to_fc(&(req->event));
    #else
    __rt_cluster_push_fc_event(&req->event);
    #endif
}

void cl_ram_free(struct pi_device *device, uint32_t chunk, uint32_t size, cl_ram_free_req_t *req)
{
    req->device = device;
    req->size = size;
    req->chunk = chunk;
    req->cid = pi_cluster_id();
    req->done = 0;
    #if defined(__PULP_OS__)
    __rt_task_init_from_cluster(&req->event);
    #endif  /* __PULP_OS__ */
    pi_task_callback(&req->event, __ram_free_cluster_req, (void* )req);
    #if defined(PMSIS_DRIVERS)
    cl_send_task_to_fc(&(req->event));
    #else
    __rt_cluster_push_fc_event(&req->event);
    #endif
}
#endif
