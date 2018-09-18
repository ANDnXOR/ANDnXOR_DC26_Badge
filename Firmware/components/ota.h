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
#ifndef COMPONENTS_OTA_H_
#define COMPONENTS_OTA_H_

#define OTA_FIRMWARE_BIN_URL "http://192.168.10.1:51337/mrmeeseeks.enc.bin"
#define OTA_FIRMWARE_BIN_FILE "/sdcard/mrmeeseeks.enc.bin"

#define OTA_FIRMWARE_SHA256_URL "http://192.168.10.1:51337/mrmeeseeks.enc.bin.sha256"
#define OTA_FIRMWARE_SHA256_FILE "/sdcard/mrmeeseeks.enc.bin.sha256"

#define OTA_FIRMWARE_VERSION_URL "http://192.168.10.1:51337/mrmeeseeks.enc.bin.version"
#define OTA_FIRMWARE_VERSION_FILE "/sdcard/mrmeeseeks.enc.bin.version"

/**
 * @brief Download the latest OTA
 * @param parameters : Task parameters, ignored
 */
extern void ota_download_task(void* parameters);

/**
 * @brief Reboot the badge into the boot stub to force an update
 */
extern void ota_reboot_and_flash();

#endif