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
#ifndef COMPONENTS_SYSTEM_H_
#define COMPONENTS_SYSTEM_H_

#include "sdkconfig.h"

//Machine readable version
#define VERSION_INT 130

//Human readable string version 
#define __BASE_VERSION__ "1.3.0"
#ifdef CONFIG_BADGE_TYPE_STANDALONE
#define VERSION __BASE_VERSION__ "S"
#elif defined(CONFIG_BADGE_TYPE_MASTER)
#define VERSION __BASE_VERSION__ "M"
#else
#define VERSION __BASE_VERSION__ ""
#endif

// Standard Includes
#include <byteswap.h>
#include <dirent.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/unistd.h>

// ESP Includes

#include "esp_system.h"
#include "driver/adc.h"
#include "driver/i2c.h" 
#include "driver/ledc.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_adc_cal.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_console.h"
#include "esp_err.h"
#include "esp_gap_ble_api.h"
#include "esp_vfs_dev.h"
#include "esp_vfs_fat.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_task_wdt.h"
#include "nvs_flash.h"
#include "apps/sntp/sntp.h"
#include "linenoise/linenoise.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "hwcrypto/aes.h"
#include "mbedtls/base64.h"
#include "mbedtls/sha256.h"
#include "rom/crc.h"
#include "sdmmc_cmd.h"
#include "argtable3/argtable3.h"

// 3rd party
#include "cJSON.h"
#include "esp_request.h"
#include "giflib/gif_lib.h"
#include "uthash.h"
#include "tpl.h"

// AND!XOR Includes
#include "util.h"
#include "battery.h"
#include "botnet/botnet.h"
#include "ble.h"
#include "boot.h"
#include "botnet/botnet.h"
#include "console.h"
#include "chip8.h"
#include "drv_config.h"
#include "drv_greenpak.h"
#include "drv_ili9225.h"
#include "drv_is31fl.h"
#include "drivers/drv_lis2de12.h"
#include "drv_sd.h"
#include "btn.h"
#include "gfx.h"
#include "botnet/botnet_ui.h"
#include "hal_i2c.h"
#include "addons.h"
#include "led.h"
#include "lolcode.h"
#include "ota.h"
#include "post.h"
#include "skifree.h"
#include "state.h"
#include "peers.h"
#include "qdbmp.h"
#include "time_manager.h"
#include "broadcast.h"
#include "ui.h"
#include "unlocks.h"
#include "ymodem.h"
#include "wifi.h"

#define INPUT_CHAR_MIN 32
#define INPUT_CHAR_MAX 126


#define TASK_PRIORITY_LOW 2
#define TASK_PRIORITY_BLE 4
#define TASK_PRIORITY_CONSOLE 3
#define TASK_PRIORITY_PLAY_BLING 3
#define TASK_PRIORITY_ADDONS 4
#define TASK_PRIORITY_MEDIUM 5
#define TASK_PRIORITY_STATE_SAVE 1
#define TASK_PRIORITY_HIGH 8

#define ANDNXOR_NVS_NAMESPACE "MRMEESEEKS"

#define STRING_ENCRYPTION_KEY "Ex8Al2iolEcwqYTK"

#endif /* COMPONENTS_SYSTEM_H_ */
