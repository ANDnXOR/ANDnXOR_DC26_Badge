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
#ifndef COMPONENTS_TIME_MANAGER_H_
#define COMPONENTS_TIME_MANAGER_H_

#define TIME_MAX_STRATUM 16
#define TIME_DEFAULT 1533093580
#define BEER_TIME_1_UTC 1533956400
#define BEER_TIME_2_UTC 1534042800

// extern void time_manager_start();

/**
 * @brief Handle a BLE advertised time
 */
extern void time_manager_advertisement_handle(
    ble_advertisement_time_t ble_time);

/**
 * @brief Start up background task that checks for beer times
 */
extern void time_manager_beer_time_task_start();

/**
 * @brief Get the current time stamp in seconds
 */
extern uint32_t time_manager_now_sec();

/**
 * @brief Set the current time of day seconds and microseconds
 * @param sec : Time now in seconds since epoc
 * @param usec : Time now micro seconds
 * @param source_stratum : The stratum of the time source
 */
extern void time_manager_set_time(uint32_t sec,
                                  uint32_t usec,
                                  uint8_t source_stratum);

/**
 * @brief Get the current stratum this badge is advertising time as
 */
extern uint8_t time_manager_stratum_get();

/**
 * @brief Set the current time stratum for the badge
 *
 * @param stratum Stratum to store
 */
extern void time_manager_stratum_set(uint8_t stratum);

/**
 * @brief Attempt to sync with a time source and stratum
 * @param seconds : Time now in seconds since epoch
 * @param useconds : Micro seconds
 * @param stratum : Stratum of the time source
 */
extern void time_manager_sync(uint32_t seconds,
                              uint32_t useconds,
                              uint8_t stratum);

#endif /* COMPONENTS_TIME_MANAGER_H_ */
