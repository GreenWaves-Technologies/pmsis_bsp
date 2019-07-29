/*
 * Copyright (C) 2019 GreenWaves Technologies
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BSP__GAPOC_A_H__
#define __BSP__GAPOC_A_H__

#define CONFIG_MT9V034
#define CONFIG_HYPERFLASH
#define CONFIG_NINA_W10
#define CONFIG_ILI9341
#define CONFIG_HYPERRAM
#define CONFIG_NINA_B112

#define CONFIG_MT9V034_CPI_ITF               0
#define CONFIG_MT9V034_I2C_ITF               1
#define CONFIG_MT9V034_POWER_GPIO            5
#define CONFIG_MT9V034_POWER_GPIO_PAD        9
#define CONFIG_MT9V034_POWER_GPIO_PAD_FUNC   1
#define CONFIG_MT9V034_TRIGGER_GPIO          3
#define CONFIG_MT9V034_TRIGGER_GPIO_PAD      7
#define CONFIG_MT9V034_TRIGGER_GPIO_PAD_FUNC 1

#define CONFIG_ILI9341_SPI_ITF           1
#define CONFIG_ILI9341_SPI_CS            0
#define CONFIG_ILI9341_GPIO              0
#define CONFIG_ILI9341_GPIO_PAD          4
#define CONFIG_ILI9341_GPIO_PAD_FUNC     1

#define CONFIG_HYPERFLASH_HYPER_ITF 0
#define CONFIG_HYPERFLASH_HYPER_CS  1

#define CONFIG_NINA_W10_SPI_ITF       1
#define CONFIG_NINA_W10_SPI_CS        0

#define CONFIG_HYPERRAM_HYPER_ITF 0
#define CONFIG_HYPERRAM_HYPER_CS  0
#define CONFIG_HYPERRAM_START     0
#define CONFIG_HYPERRAM_SIZE     (1<<20)

#define CONFIG_HYPERBUS_DATA6_PAD           38
// This is due to a HW bug, to be fixed in the future
#define CONFIG_UART_RX_PAD_FUNC             0
#define CONFIG_HYPERRAM_DATA6_PAD_FUNC      3

#define GPIOA0_LED                0
#define GPIOA1                    1
#define GPIOA2_NINA_RST           2
#define GPIOA3_CIS_EXP            3
#define GPIOA4_1V8_EN             4
#define GPIOA5_CIS_PWRON          5
#define GPIOA18                   18
#define GPIOA19                   19
#define GPIOA21_NINA17            21


void board_init();

#endif
