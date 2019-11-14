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

#ifndef __BSP_GAPOC_B_H__
#define __BSP_GAPOC_B_H__

#define CONFIG_THERMAL_EYE
#define CONFIG_HYPERFLASH
#define CONFIG_ILI9341
#define CONFIG_HYPERRAM
#define CONFIG_NINA_B112

#define CONFIG_ILI9341_SPI_ITF         ( 1 )
#define CONFIG_ILI9341_SPI_CS          ( 0 )
#define CONFIG_ILI9341_GPIO            ( 0 )
#define CONFIG_ILI9341_GPIO_PAD        ( PI_PAD_12_A3_RF_PACTRL0 )
#define CONFIG_ILI9341_GPIO_PAD_FUNC   ( PI_PAD_12_A3_GPIO_A0_FUNC1 )

#define CONFIG_HYPERFLASH_HYPER_ITF    ( 0 )
#define CONFIG_HYPERFLASH_HYPER_CS     ( 1 )

#define CONFIG_HYPERRAM_HYPER_ITF      ( 0 )
#define CONFIG_HYPERRAM_HYPER_CS       ( 0 )
#define CONFIG_HYPERRAM_START          ( 0 )
#define CONFIG_HYPERRAM_SIZE           ( 8 << 20 )

#define CONFIG_HYPERBUS_DATA6_PAD      ( PI_PAD_46_B7_SPIM0_SCK )
// This is due to a HW bug, to be fixed in the future
#define CONFIG_UART_RX_PAD_FUNC        ( 0 )
#define CONFIG_HYPERRAM_DATA6_PAD_FUNC ( 3 )

/* PWM 3 channel 3. */
#define CONFIG_PWM3CH3_IR_CLK           ( PI_PAD_29_B34_CAM_SDA )
#define CONFIG_PWM3CH3_FUNC             ( PI_PAD_29_B34_TIMER3_CH3_FUNC2 )

#define GPIOA0_LED                     ( PI_GPIO_A1_PAD_13_B2 )
#define GPIOA1                         ( 1 )
#define GPIOA2_NINA_RST                ( PI_GPIO_A2_PAD_14_A2 )
#define GPIOA3_CIS_EXP                 ( PI_GPIO_A3_PAD_15_B1 )
#define GPIOA4_1V8_EN                  ( PI_GPIO_A4_PAD_16_A44 )
#define GPIOA5_CIS_PWRON               ( PI_GPIO_A5_PAD_17_B40 )
#define GPIO_IR_TRIG                   ( PI_GPIO_A3_PAD_15_B1 )
#define GPIO_IR_PWRON                  ( PI_GPIO_A5_PAD_17_B40 )
#define GPIO_IR_NRST                   ( PI_GPIO_A16_PAD_30_D1 )
//#define GPIO_PWM3CH3_IR_CLK            ( PI_GPIO_)
#define GPIOA18                        ( 18 )
#define GPIOA19                        ( 19 )
#define GPIOA21_NINA17                 ( PI_GPIO_A21_PAD_35_B13 )


void board_init();

#endif  /* __BSP_GAPOC_B_H__ */
