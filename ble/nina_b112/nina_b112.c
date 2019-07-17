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

//#include "stddef.h"
#include <string.h>
#include "pmsis.h"
//#include "pmsis_l2_malloc.h"
//#include "pmsis_task.h"
//#include "uart.h"
//#include "udma_uart.h"
#include "bsp/bsp.h"
#include "bsp/ble/nina_b112/nina_b112.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

typedef enum
{
    WR_RES_OK = 0,
    WR_RES_ERR = -1,
    WR_RES_UNSOL = -2,
    WR_RES_NA = -3
} write_res_t;

typedef struct
{
    nina_ble_t *ble;
    struct pi_task *task;
} cb_args;

static cb_args param = {0};

static volatile at_resp_state_t at_resp_state;

/*******************************************************************************
 * API implementation
 ******************************************************************************/

void nina_b112_init_conf(nina_ble_t *ble)
{
}

int32_t nina_b112_open(nina_ble_t *ble)
{
    struct pi_uart_conf *uart_conf = pmsis_l2_malloc(sizeof(struct pi_uart_conf));
    if (uart_conf == NULL)
    {
        printf("Uart conf null\n");
        return -1;
    }
    pi_uart_conf_init(uart_conf);
    uart_conf->baudrate_bps = (uint32_t) NINA_UART_AT_BAUDRATE_bps;
    /* Format 8N-0-1. */
    //uart_conf->bit_length = UART_8_BITS;
    //uart_conf->parity_mode = UART_PARITY_DISABLED;
    //uart_conf->stop_bit_count = UART_ONE_STOP_BIT;
    uart_conf->enable_rx = 1;
    uart_conf->enable_tx = 1;
#ifdef PMSIS_DRIVER
    uart_conf->src_clock_Hz = SystemCoreClock;
#endif

    struct nina_b112_conf *_conf = pmsis_l2_malloc(sizeof(struct nina_b112_conf));
    _conf->skip_pads_config = 0;
    bsp_nina_b112_open(_conf);

    ble->rx_char = (uint8_t *) pmsis_l2_malloc(sizeof(uint8_t));
    if (ble->rx_char == NULL)
    {
        printf("Char null\n");
        return -2;
    }
    ble->rx_buffer = (uint8_t *) pmsis_l2_malloc(sizeof(uint8_t) * (uint32_t) AT_RESP_ARRAY_LENGTH);
    if (ble->rx_buffer == NULL)
    {
        printf("Buffer null\n");
        return -3;
    }
    else
    {
        for (uint32_t i=0; i<(uint32_t) AT_RESP_ARRAY_LENGTH; i++)
        {
            ble->rx_buffer[i] = 0x0;
        }
    }
    ble->uart_device.config = (void *) uart_conf;
    if (pi_uart_open(&(ble->uart_device)))
    {
        return -4;
    }
    return 0;
}

void nina_b112_close(nina_ble_t *ble)
{
    pi_uart_close(&(ble->uart_device));
    pmsis_l2_malloc_free(ble->rx_char, sizeof(uint8_t));
    pmsis_l2_malloc_free(ble->rx_buffer, sizeof(uint8_t) * (uint32_t) AT_RESP_ARRAY_LENGTH);
    pmsis_l2_malloc_free(ble->uart_device.config, sizeof(struct pi_uart_conf));
}

void nina_b112_AT_cmd_send(nina_ble_t *ble, const char* cmd)
{
    uint32_t cmd_length = strlen(cmd);
    uint32_t length = cmd_length + 2 + 1; /* cmd length + "AT" + '\r'. */
    char *cmd_string = (char *) pmsis_l2_malloc(sizeof(char) * length);
    if (cmd_string == NULL)
    {
        return;
    }
    strcpy((char*) cmd_string, (char*) "AT");
    strcat((char*) cmd_string, (char*) cmd);
    strcat((char*) cmd_string, (char*) S3str);
    pi_uart_write(&(ble->uart_device), cmd_string, strlen(cmd_string));
    pmsis_l2_malloc_free(cmd_string, sizeof(char) * length);
}

/* Uart rx callback. */
static void __nina_b112_data_received(void *arg)
{
    cb_args *param = (cb_args *) arg;
    static uint32_t index = 0;
    static unsigned char prev_byte = 0;
    if ((at_resp_state == AT_RESP_IN_PROGRESS) &&
        (prev_byte == S3char) && (*(param->ble->rx_char) == S4char))
    {
        param->ble->rx_buffer[--index] = '\0';
        at_resp_state = AT_RESP_DONE;
    }
    else
    {
        pi_task_callback(param->task, __nina_b112_data_received, param);
        pi_uart_read_async(&(param->ble->uart_device), (void *) param->ble->rx_char, sizeof(uint8_t), param->task);
        if ((at_resp_state == AT_RESP_NOT_STARTED) &&
            (prev_byte == S3char) && (*(param->ble->rx_char) == S4char))
        {
            index = 0;
            at_resp_state = AT_RESP_IN_PROGRESS;
        }
        else if (at_resp_state == AT_RESP_IN_PROGRESS)
        {
            param->ble->rx_buffer[index++] = *(param->ble->rx_char);
        }
    }
    prev_byte = *(param->ble->rx_char);
}

uint32_t nina_b112_AT_send(nina_ble_t *ble, const char* cmd)
{
    at_resp_state = AT_RESP_NOT_STARTED;
    pi_task_t rx_cb = {0};
    param.ble = ble;
    param.task = &rx_cb;
    pi_task_callback(&rx_cb, __nina_b112_data_received, &param);
    pi_uart_read_async(&(ble->uart_device), (void *) ble->rx_char, sizeof(uint8_t), &rx_cb);
    nina_b112_AT_cmd_send(ble, cmd);

    write_res_t write_result = WR_RES_NA;
    while (at_resp_state != AT_RESP_DONE)
    {
        pi_yield();
    }
    DBG_PRINT("Got write resp : %s\n", ble->rx_buffer);

    uint32_t last_char_pos = strlen((const char *) ble->rx_buffer) - 1;
    if ((ble->rx_buffer[last_char_pos - 1] == 'O') &&
        (ble->rx_buffer[last_char_pos - 0] == 'K'))
    {
        DBG_PRINT("OK response received !\n");
        write_result = WR_RES_OK;
    }
    else if ((ble->rx_buffer[last_char_pos - 4] == 'E') &&
             (ble->rx_buffer[last_char_pos - 3] == 'R') &&
             (ble->rx_buffer[last_char_pos - 2] == 'R') &&
             (ble->rx_buffer[last_char_pos - 1] == 'O') &&
             (ble->rx_buffer[last_char_pos - 0] == 'R'))
    {
        DBG_PRINT("Error response received !\n");
        write_result = WR_RES_ERR;
    }
    else
    {
        DBG_PRINT("Unsollicited/unrecognised response received : %s !\n",
                  ble->rx_buffer);
        write_result = WR_RES_UNSOL;
    }
    return write_result;
}

void nina_b112_AT_query(nina_ble_t *ble, const char* cmd, char* resp)
{
    at_resp_state = AT_RESP_NOT_STARTED;
    pi_task_t rx_cb = {0};
    param.ble = ble;
    param.task = &rx_cb;
    pi_task_callback(&rx_cb, __nina_b112_data_received, &param);
    pi_uart_read_async(&(ble->uart_device), (void *) ble->rx_char, sizeof(uint8_t), &rx_cb);
    nina_b112_AT_cmd_send(ble, cmd);

    while(at_resp_state != AT_RESP_DONE)
    {
        pi_yield();
    }
    strcpy((char *) resp, (char *) ble->rx_buffer);
    DBG_PRINT("Got query resp : %s\n", resp);
}

void nina_b112_wait_for_event(nina_ble_t *ble, char* resp)
{
    at_resp_state = AT_RESP_NOT_STARTED;
    pi_task_t rx_cb = {0};
    param.ble = ble;
    param.task = &rx_cb;
    pi_task_callback(&rx_cb, __nina_b112_data_received, &param);
    pi_uart_read_async(&(ble->uart_device), (void *) ble->rx_char, sizeof(uint8_t), &rx_cb);

    while (at_resp_state != AT_RESP_DONE)
    {
        pi_yield();
    }
    strcpy((char *) resp, (char *) ble->rx_buffer);
    DBG_PRINT("Got unsollicited resp : %s\n", resp);
}

void nina_b112_get_data_blocking(nina_ble_t *ble, uint8_t* buffer, uint32_t size)
{
    pi_uart_read(&(ble->uart_device), buffer, size);
}

void nina_b112_get_data(nina_ble_t *ble, uint8_t* buffer, uint32_t size, struct pi_task *task)
{
    pi_uart_read_async(&(ble->uart_device), buffer, size, task);
}

void nina_b112_send_data_blocking(nina_ble_t *ble, const uint8_t* buffer, uint32_t size)
{
    pi_uart_write(&(ble->uart_device), (void *) buffer, size);
}

void nina_b112_send_data(nina_ble_t *ble, const uint8_t* buffer, uint32_t size, struct pi_task *task)
{
    pi_uart_write_async(&(ble->uart_device), (void *) buffer, size, task);
}

void nina_b112_get_data_async(nina_ble_t *ble, uint8_t* buffer, uint32_t size,
                              ble_callback_f callback, void *arg)
{
    pi_task_t rx_cb = {0};
    pi_task_callback(&rx_cb, callback, arg);
    pi_uart_read_async(&(ble->uart_device), buffer, size, &rx_cb);
}

void nina_b112_send_data_async(nina_ble_t *ble, const uint8_t* buffer, uint32_t size,
                               ble_callback_f callback, void *arg)
{
    pi_task_t rx_cb = {0};
    pi_task_callback(&rx_cb, callback, arg);
    pi_uart_write_async(&(ble->uart_device), (void *) buffer, size, &rx_cb);
}

/*
 * Escape data mode.
 * Note : a delay of 1s is needed before and after sending this cmd.
 */
void nina_b112_exit_data_mode(nina_ble_t *ble)
{
    pi_uart_write(&(ble->uart_device), (void *) '+', 1);
    pi_uart_write(&(ble->uart_device), (void *) '+', 1);
    pi_uart_write(&(ble->uart_device), (void *) '+', 1);
}

