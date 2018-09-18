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

static const char* TAG = "MRMEESEEKS::Time_Manager";
uint8_t m_stratum = TIME_MAX_STRATUM;

/**
 * @brief Low priority task that occasionally checks if it's beer time, if so
 * notify the user!
 */
static void __beer_time_task() {
  while (1) {
    uint32_t now = time_manager_now_sec();
    // Test for beer time
    ESP_LOGD(TAG, "%sBeer time? Now=%u Time 1=%d Time 2=%d", LOG_YELLOW, now,
             BEER_TIME_1_UTC, BEER_TIME_2_UTC);
    if ((now > BEER_TIME_1_UTC && now < (BEER_TIME_1_UTC + 60)) ||
        (now > BEER_TIME_2_UTC && now < (BEER_TIME_2_UTC + 60))) {
      ui_interrupt("IT'S BEER TIME!", "/sdcard/bling/beertime.raw", 60);
    }

    // Wait to test again
    DELAY(20000);
  }

  // We're past beer times :(
  vTaskDelete(NULL);
}

/**
 * @brief Handle a BLE advertised time
 */
void time_manager_advertisement_handle(ble_advertisement_time_t ble_time) {
  // First thing, check the CRC
  uint16_t crc =
      crc16_le(0, (uint8_t*)&ble_time, sizeof(ble_advertisement_time_t) - 2);
  if (crc != *(uint16_t*)ble_time.crc) {
    ESP_LOGD(TAG, "%s CRC Invalid :(", LOG_RED);
    return;
  }

  if (ble_time.canary != BLE_TIME_CANARY) {
    return;
  }

  // If we got this far, it is a valid advertisement, try to sync internal time
  // with it
  time_manager_sync(*(uint32_t*)ble_time.seconds, *(uint32_t*)ble_time.useconds,
                    ble_time.stratum);
}

/**
 * @brief Start up background task that checks for beer times
 */
void time_manager_beer_time_task_start() {
  static StaticTask_t task;
  util_task_create(__beer_time_task, "Beer Time", 4096, NULL, TASK_PRIORITY_LOW,
                   &task);
}

/**
 * @brief Attempt to sync with a time source and stratum
 * @param seconds : Time now in seconds since epoch
 * @param useconds : Micro seconds
 * @param stratum : Stratum of the time source
 */
void time_manager_sync(uint32_t seconds, uint32_t useconds, uint8_t stratum) {
  struct timeval time;
  gettimeofday(&time, NULL);

  bool better_time_source = false;
  bool later = false;

  // Determine if the time source is later than us
  if (seconds > time.tv_sec) {
    later = true;
  } else {
    later = (seconds == time.tv_sec) && (useconds > time.tv_usec);
  }

  // Time source is in the past, ignore always
  if (!later) {
    return;
  }

  // Start comparing stratums. Jump if 1) better stratum or 2) Both
  // unsynchronized
  if (stratum < m_stratum) {
    better_time_source = true;
  } else {
    better_time_source =
        (m_stratum == TIME_MAX_STRATUM) && (stratum = TIME_MAX_STRATUM);
  }

  // Jump to their time
  if (better_time_source) {
    time_manager_set_time(seconds, useconds, stratum);
  }
}

/**
 * @brief Get the current time stamp in seconds
 */
uint32_t time_manager_now_sec() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}

/**
 * @brief Set the current time of day seconds and microseconds
 * @param sec : Time now in seconds since epoc
 * @param usec : Time now micro seconds
 * @param source_stratum : The stratum of the time source
 */
void time_manager_set_time(uint32_t sec,
                           uint32_t usec,
                           uint8_t source_stratum) {
  // Avoid 2038 problem
  if (sec >= 2147483647) {
    return;
  }

  struct timeval time;
  time.tv_sec = sec;
  time.tv_usec = usec;
  settimeofday(&time, NULL);

  ESP_LOGD(TAG, "%sSetting time to %u.%u [Stratum %d]", LOG_YELLOW, sec, usec,
           source_stratum);

  time_manager_stratum_set(source_stratum + 1);
}

/**
 * @brief Get the current stratum this badge is advertising time as
 */
uint8_t time_manager_stratum_get() {
  return m_stratum;
}

/**
 * @brief Set the current time stratum for the badge
 *
 * @param stratum Stratum to store
 */
void time_manager_stratum_set(uint8_t stratum) {
  m_stratum = MIN(TIME_MAX_STRATUM, stratum);
}