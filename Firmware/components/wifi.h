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
#ifndef COMPONENTS_WIFI_H_
#define COMPONENTS_WIFI_H_

#define USER_AGENT "Mozilla/3.01Gold (AND!XOR; DC26; esp32/mrmeeseeks; v"VERSION")"

/**
 * @brief Initialize the wifi module but don't connect
 */
extern void wifi_initialize();

/**
 * @brief Get connected state of wifi
 * @return true if currently connected
 */
extern bool wifi_is_connected();

/**
 * Generate a soft AP with a given SSID
 * @param ssid : Name of the AP to run
 */
extern void wifi_soft_ap(const char* ssid);

/**
 * Stop the soft AP
 */
extern void wifi_soft_ap_stop();

/**
 * @brief Start the wifi module and connect to the given SSID
 * @param wc : Wifi config to use for the connection
 */
extern void wifi_start(wifi_config_t wc);

/**
 * @brief Stop the wifi module and return badge to previous state
 */
extern void wifi_stop();

#endif /* COMPONENTS_WIFI_H_ */
