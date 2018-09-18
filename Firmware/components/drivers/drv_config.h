/*****************************************************************************
 * Made with beer and late nights in California.
 *
 * (C) Copyright 2017-2018 AND!XOR LLC (http://andnxor.com/).
 *
 * PROPRIETARY AND CONFIDENTIAL UNTIL AUGUST 7th, 2018 then,
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
 *
 * ADDITIONALLY:
 * If you find this source code useful in anyway, use it in another electronic
 * conference badge, or just think it's neat. Consider buying us a beer
 * (or two) and/or a badge (or two). We are just as obsessed with collecting
 * badges as we are in making them.
 *
 * Contributors:
 * 	@andnxor
 * 	@zappbrandnxor
 * 	@hyr0n1
 * 	@exc3ls1or
 * 	@lacosteaef
 * 	@bitstr3m
 *****************************************************************************/
#include "system.h"

#ifndef BUILD_DRV_CONFIG_H_
#define BUILD_DRV_CONFIG_H_

#define LCD_PIN_MOSI 23
#define LCD_PIN_CLK 18
#define LCD_PIN_CS 5
#define LCD_PIN_DC 22

#ifndef CONFIG_BADGE_TYPE_STANDALONE
#define LCD_PIN_RST 19
#define LED_EYE_PIN 21
#endif

#define I2C_1_FREQ_HZ 400000
#define I2C_2_FREQ_HZ 400000
#define I2C_1_SDA 25
#define I2C_1_SCL 33
#define I2C_2_SDA 27
#define I2C_2_SCL 26
#define I2C_MASTER_RX_BUF_DISABLE 0
#define I2C_MASTER_TX_BUF_DISABLE 0
#define I2C_MAX_DELAY 100

#define LED_I2C_ADDR ISSI_ADDR_DEFAULT
#define LED_I2C_MASTER_NUM I2C_NUM_1

#define ACCEL_I2C_ADDR_1 0x18
#define ACCEL_I2C_ADDR_2 0x19
#define ACCEL_I2C_MASTER_NUM I2C_NUM_0
#define ACCEL_ISR_GPIO 35
#define ACCEL_ISR_GPIO_MASK GPIO_SEL_35

#define GREENPAK_I2C_ADDR 0x30
#define GREENPAK_I2C_MASTER_NUM I2C_NUM_0

#define ADDON_I2C_MASTER_NUM I2C_NUM_0

// SD Card in SPI mode
#define SD_PIN_MISO 2
#define SD_PIN_MOSI 15
#define SD_PIN_CLK 14
#define SD_PIN_CS 13

#define SD_CARD_DETECT 39

#endif /* BUILD_DRV_CONFIG_H_ */
