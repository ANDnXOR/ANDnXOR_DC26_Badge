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

#ifndef COMPONENTS_STATE_H_
#define COMPONENTS_STATE_H_

#define STATE_FILE "/sdcard/badge.state"

#define STATE_BRIGHTNESS_MIN 1
#define STATE_BRIGHTNESS_MAX 5
#define STATE_BRIGHTNESS_DEFAULT 3

#define STATE_NAME_LENGTH 8
#define STATE_NAME_DEFAULT ""

#define STATE_BADGELIFE_MAGIC_BYTE_DEFAULT 0

#define STATE_TILT_ENABLED_DEFAULT true

#define STATE_UNLOCK_DEFAULT 0

typedef struct {
  char name[STATE_NAME_LENGTH + 1];  //+1 for null termination
  bool airplane_mode_enabled;
  uint8_t brightness;
  bool tilt_enabled;
  uint16_t unlock_state;
  bool master_badge;
  botnet_state_t botnet_state;
  uint32_t time_sec;
  uint32_t score_pong;
  uint32_t score_ski;
  uint8_t badgelife_magic_byte;
  bool hide_bling;
  game_data_t console_state;
  uint8_t padding[4]; //make sure it's aligned on 128-bit boundary
  uint32_t crc;
} badge_state_t;

/**
 * Initilize badge state handler. This does not load any data, merely gets
 * things ready
 */
extern void state_init();
extern botnet_state_t* state_botnet_get();
extern game_data_t* state_console_get();

/**
 * Load the badge state from NVS storage.
 *
 * NOTE: This should be done from the main task or a task with at least low
 * priority (not idle)
 */
extern void state_load();

/**
 * @brief Get the current badgelife magic byte
 */
extern uint8_t state_badgelife_magic_byte_get();

/**
 * @brief Set the badgelife magic byte
 */
extern void state_badgelife_magic_byte_set(uint8_t magic_byte);
extern uint8_t state_brightness_get();
extern void state_brightness_set(uint8_t brightness);
extern bool state_hide_bling_get();
extern void state_hide_bling_set(bool hide);
extern void state_name_get(char* name);
extern void state_name_set(char* name);

/**
 * @brief Reset badge state to new
 */
extern void state_new();

/**
 * @brief Indicate to badge that something has changed in state and it needs
 * saving
 */
extern void state_save_indicate();

/**
 * @brief Run a task in the background to handle saving state (also saves peers)
 */
extern void state_save_task_start();

/**
 * @brief Get the current high pong score
 */
extern uint32_t state_score_pong_get();

/**
 * @brief Set the current high pong score
 */
extern void state_score_pong_set(uint32_t score);

/**
 * @brief Get the current high ski score
 */
extern uint32_t state_score_ski_get();

/**
 * Set the curren high ski free score
 */
extern void state_score_ski_set(uint32_t score);

/**
 * @brief Get the current tilt enabled state
 */
extern bool state_tilt_get();

/**
 * @brief Enable or disable the screen tilt
 */
extern void state_tilt_set(bool tilt);

/**
 * @brief Get the current unlock state
 * @return Current unlock state as a bitmask
 */
extern uint16_t state_unlock_get();

/**
 * @brief Set the current unlock state
 * @param unlock : The unlock state as a bitmask
 */
extern void state_unlock_set(uint16_t unlock);

#endif
