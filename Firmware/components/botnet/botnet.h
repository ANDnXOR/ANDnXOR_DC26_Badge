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
#pragma once
#include "system.h"

#ifndef COMPONENTS_BOTNET_BOTNET_H_
#define COMPONENTS_BOTNET_BOTNET_H_

#define BOTNET_ID_LENGTH 4
#define BOTNET_PWNED_BY_LENGTH 8
#define BOTNET_DEFAULT_EXPERIENCE 0
#define BOTNET_DEFAULT_LEVEL 1
#define BOTNET_DEFAULT_POINTS 100
#define BOTNET_DEFAULT_STRENGTH 0

#define BOTNET_LEVEL_MAX 70
#define BOTNET_MAX_STRENGTH 255
#define BOTNET_STRENGTH_PER_LEVEL 6

// Define game parameters. These must be divisible by 10 since points
// and XP etc are evaluated 10 times per hour
#define BOTNET_POINTS_PER_HOUR 100
#define BOTNET_XP_PER_PEER_PER_HOUR 10
#define BOTNET_CLEAN_COST 100
#define BOTNET_RESEARCH_COST 10
#define BOTNET_STRENGTH_DEGRADE_PER_STEP 0.75
#define BOTNET_STRENGHT_DEGRADE_PER_HOP 10

// Durations
#define BOTNET_HIDE_BLING_TIME_MS (5 * 60 * 1000)
#define BOTNET_REVERSE_SCREEN_TIME_MS (5 * 60 * 1000)

// XP required per level to move up
#define BOTNET_SP_PER_LEVEL_REQD 100

typedef enum {
  botnet_payload_none, /* No payload */

  botnet_payload_help,
  botnet_payload_join,
  botnet_payload_clean,
  botnet_payload_bling,
  botnet_payload_history_poison,
  botnet_payload_invert_colors,
  botnet_payload_reverse_screen,
  botnet_payload_buttons,
  botnet_payload_quit,
  botnet_payload_no_bling,
  botnet_payload_unlocks_delete,
  botnet_payload_ap_rick_roll,
  botnet_payload_ap_d4rkm4tter,
  botnet_payload_badgelife_mimic,

  botnet_payload_reset, /* Remove/reset infection */
  botnet_payload_count  /* Counter of payloads */
} botnet_payload_t;

typedef enum {
  botnet_payload_type_nice,
  botnet_payload_type_mean,
  botnet_payload_type_master
} botnet_payload_type_t;

__attribute__((unused)) static const char* botnet_payload_str[] = {
    "None",
    "Help",
    "Join",
    "Clean",
    "Bling",
    "Console Poison",
    "Invert Colors",
    "Reverse Screen",
    "Random Buttons",
    "Leave Botnet",
    "No Bling",
    "Delete Unlocks",
    "Rick Roll AP",
    "D4RKM4TTER AP",
    "Masquerade",
    "Master Reset",
    "None"};

static const botnet_payload_type_t botnet_payload_types[] = {
    botnet_payload_type_master,  // none

    botnet_payload_type_nice,  // help
    botnet_payload_type_mean,  // join
    botnet_payload_type_nice,  // clean
    botnet_payload_type_mean,  // bling
    botnet_payload_type_mean,  // history
    botnet_payload_type_mean,  // invert
    botnet_payload_type_mean,  // reverse
    botnet_payload_type_mean,  // buttons
    botnet_payload_type_nice,  // quit
    botnet_payload_type_mean,  // no blings
    botnet_payload_type_mean,  // unlocks
    botnet_payload_type_mean,  // ap rick
    botnet_payload_type_mean,  // ap d4rk
    botnet_payload_type_mean,  // mimic

    botnet_payload_type_master   // reset
};

typedef enum {
  botnet_bling_rick_roll,
  botnet_bling_hypnotoad,
  botnet_bling_bsod,
  botnet_bling_wannacry,
  botnet_bling_clippy,
  botnet_bling_rick_morty,

  botnet_bling_count
} botnet_bling_t;

__attribute__((unused)) const static char* botnet_bling_paths[] = {
    "/sdcard/bling/rick.raw",   "/sdcard/bling/hypnotoad1.raw",
    "/sdcard/bling/bsod.raw",   "/sdcard/bling/wannacry.raw",
    "/sdcard/bling/clippy.raw", "/sdcard/bling/rm1.raw",
};

__attribute__((unused)) const static char* botnet_bling_labels[] = {
    "Rick Roll", "Hypnotoad", "BSOD", "WannaCry", "Clippy", "Rick & Morty"};

typedef enum {
  botnet_console_poison_rick_roll,
  botnet_console_poison_pwned_by,

  botnet_console_poison_count
} botnet_console_poison_t;

typedef struct {
  uint8_t level;
  uint16_t experience;
  uint16_t points;
  uint16_t botnet_id;
  bool first_run;
} botnet_state_t;

// Structure for holding the botnet packet advertised over GAP.
// This must be exactly 16 bytes to support AES ECB
typedef struct {
  uint8_t payload;
  uint8_t data;
  uint8_t strength;
  uint8_t timestamp[4];
  uint8_t serial[2];
  uint8_t padding[5];
  uint8_t crc[2];  // This must be last
} ble_advertisement_botnet_t;

/**
 * Handle botnet advertisements individually
 *
 * @param packet	Pointer to packet that we need to parse/handle
 */
extern void botnet_advertiser_handler(ble_advertisement_botnet_t* packet);

/**
 * Get a pointer to the current botnet packet to advertise
 *
 * @return a pointer to the current botnet packet
 */
extern ble_advertisement_botnet_t* botnet_packet_get();

/**
 * @brief Initialize the botnet
 */
extern void botnet_init();

/**
 * @brief Level up the player
 */
extern void botnet_level_up();

/**
 * @brief : Set a botnet packet. This includes setting the CRC
 * @param p_packet : Pointer to botnet packet to set
 */
extern void botnet_packet_set(ble_advertisement_botnet_t* p_packet);

/**
 * @brief Validate a given botnet packet
 * @param p_packet : Pointer to the packet to validate
 * @return True if valid
 */
extern bool botnet_payload_validate(ble_advertisement_botnet_t* p_packet);

/**
 * @brief Reset botnet state, careful player will lose all progress!
 */
extern void botnet_reset();

#endif /* COMPONENTS_BOTNET_BOTNET_H_ */
