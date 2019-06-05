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

#ifndef __BSP__GAPOC_A_H__
#define __BSP__GAPOC_A_H__

#define CONFIG_MT9V034
#define CONFIG_HYPERFLASH

#define CONFIG_MT9V034_CPI_ITF               0
#define CONFIG_MT9V034_I2C_ITF               1
#define CONFIG_MT9V034_POWER_GPIO            5
#define CONFIG_MT9V034_POWER_GPIO_PAD        PAD_17_RF_PACTRL5
#define CONFIG_MT9V034_POWER_GPIO_PAD_FUNC   PAD_17_FUNC1_GPIOA5
#define CONFIG_MT9V034_TRIGGER_GPIO          3
#define CONFIG_MT9V034_TRIGGER_GPIO_PAD      PAD_15_RF_PACTRL3
#define CONFIG_MT9V034_TRIGGER_GPIO_PAD_FUNC PAD_15_FUNC1_GPIOA3

#define CONFIG_HYPERFLASH_HYPER_ITF 0
#define CONFIG_HYPERFLASH_HYPER_CS  1

#endif