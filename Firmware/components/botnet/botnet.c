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
#include "rick.h"
#include "system.h"
const static char* TAG = "MRMEESEEKS::Botnet";

// Default botnet packet advertised should be innocuous
static ble_advertisement_botnet_t m_botnet_packet = {
    //	.payload = botnet_payload_none,
    //	.data = 0x00,
    //	.ttl = 0,
    //	.strength = 0,
    //	.pwned_by = ""
    .payload = 0,
    .data = 0,
    .strength = BOTNET_DEFAULT_STRENGTH,
    .timestamp = {0, 0, 0, 0},
    .serial = {0x66, 0x66},
};

static botnet_payload_t m_current_payload = botnet_payload_none;
static bool m_payload_stop = false;
static uint32_t m_timestamp = 0;
static xQueueHandle m_payload_queue = NULL;

/**
 * @brief Primary background botnet task that handles points, XP, level up, etc
 * @param parameters : Task parameters, ignored.
 */
static void __botnet_task(void* parameters) {
  // Do the background botnet task every  6 minutes
  TickType_t start = xTaskGetTickCount();
  TickType_t ticks = (6 * 60 * 1000) / portTICK_PERIOD_MS;
  while (1) {
    vTaskDelayUntil(&start, ticks);
    start = xTaskGetTickCount();
    ESP_LOGD(TAG, "Botnet background task");

    botnet_state_t* p_state = state_botnet_get();

    // Botnet strength decreases by 10 per hour
    float strength = botnet_packet_get()->strength;
    strength = strength * BOTNET_STRENGTH_DEGRADE_PER_STEP;
    botnet_packet_get()->strength = (uint8_t)strength;

    // Points increase by level * 100 per hour
    p_state->points += (BOTNET_POINTS_PER_HOUR / 10 * p_state->level);

    // XP increase by (level + peer count) * 10 per hour
    p_state->experience +=
        (p_state->level + peers_count()) * (BOTNET_XP_PER_PEER_PER_HOUR / 10);

    // Check for level up
    while (p_state->experience >= (p_state->level * BOTNET_SP_PER_LEVEL_REQD)) {
      // level up
      p_state->experience -= (p_state->level * BOTNET_SP_PER_LEVEL_REQD);
      p_state->level++;
    }

    // Tell state something changed
    state_save_indicate();

    ESP_LOGD(TAG, "Botnet state, level=%d points=%d XP=%d", p_state->level,
             p_state->points, p_state->experience);
  }

  // Just in case?
  vTaskDelete(NULL);
}

static IRAM_ATTR void __payload_ap_rick_roll() {
  ESP_LOGD(TAG, "%sRick Roll AP", LOG_RED);
  uint32_t end = MILLIS() + CONFIG_BOTNET_AP_RICK_ROLL_TIME;
  while (MILLIS() < end && !m_payload_stop) {
    uint8_t index = util_random(0, rick_roll_lyrics_count);
    wifi_soft_ap(rick_roll_lyrics[index]);
    ESP_LOGD(TAG, "%sSoft AP Rick Roll '%s'", LOG_CYAN,
             rick_roll_lyrics[index]);
    DELAY(CONFIG_BOTNET_AP_RICK_ROLL_ROTATION_TIME);
  }
  wifi_soft_ap_stop();
  ESP_LOGD(TAG, "%sSoft AP Rick Roll done", LOG_CYAN);
}

static IRAM_ATTR void __payload_ap_d4rkm4tter() {
  ESP_LOGD(TAG, "%sd4rkm4tter AP", LOG_RED);
  wifi_soft_ap("COME AT ME D4RKM4TTER");
  DELAY(CONFIG_BOTNET_AP_RICK_ROLL_TIME);
  wifi_soft_ap_stop();
  ESP_LOGD(TAG, "%sd4rkm4tter done", LOG_CYAN);
}

static void __payload_bling(botnet_bling_t bling) {
  ESP_LOGD(TAG, "%sPlaying bling", LOG_RED);

#ifndef CONFIG_BADGE_TYPE_STANDALONE
  // Store screen buffer to be restored when bling stops
  void* buffer = util_heap_alloc_ext(LCD_BUFFER_SIZE);
  gfx_screen_buffer_copy(buffer);

  // Play bling
  if (bling < botnet_bling_count) {
    gfx_play_raw_file(botnet_bling_paths[bling], 0, 0, LCD_WIDTH, LCD_HEIGHT,
                      NULL, true, NULL);
  }

  // restore screen buffer
  gfx_screen_buffer_restore(buffer);
  gfx_push_screen_buffer();
  free(buffer);
#endif
}

/**
 * @brief Execute randomize buttons payload
 */
static void __payload_buttons() {
  ESP_LOGD(TAG, "%sRandomizing Buttons", LOG_BLUE);
  btn_randomize(true);
  DELAY(CONFIG_BOTNET_RANDOM_BUTTONS_TIME);
  btn_randomize(false);
  ESP_LOGD(TAG, "%sDone randomizing buttons", LOG_BLUE);
}

/**
 * @brief Execute botnet payload clean which removes existing botnet payloads
 */
static void __payload_clean() {
  // If bling is being run, stop it
  if (m_current_payload == botnet_payload_bling) {
    gfx_stop();
  }
}

static void __payload_history(ble_advertisement_botnet_t* p_packet) {
  ESP_LOGD(TAG, "%sHistory Poison", LOG_RED);
  if (!drv_sd_mounted()) {
    return;
  }

  botnet_console_poison_t poison = p_packet->data;

  // Brute force clear console history
  unlink(CONSOLE_HISTORY_PATH);
  linenoiseHistoryLoad(CONSOLE_HISTORY_PATH);

  switch (poison) {
    case botnet_console_poison_pwned_by:;
      ;
      char pwned_by[32];
      sprintf(pwned_by, "PWNed by 0x%02x", *(uint16_t*)p_packet->serial);

      for (uint8_t i = 0; i < CONSOLE_HISTORY_LEN; i++) {
        linenoiseHistoryAdd(pwned_by);
      }
      linenoiseHistorySave(CONSOLE_HISTORY_PATH);
      ESP_LOGD(TAG, "%s%s console history!", LOG_CYAN, pwned_by);
      break;
    case botnet_console_poison_rick_roll:;

      // Fill console history with rick roll
      uint8_t row = 0;
      for (uint8_t i = 0; i < CONSOLE_HISTORY_LEN; i++) {
        const char* line = rick_roll_lyrics[row];
        linenoiseHistoryAdd(line);
        row = (row + 1) % rick_roll_lyrics_count;
      }
      linenoiseHistorySave(CONSOLE_HISTORY_PATH);
      ESP_LOGD(TAG, "%sRick rolled console history!", LOG_CYAN);

      break;
    case botnet_console_poison_count:
      break;
  }
}

/**
 * @brief Botnet payload that inverts the byte order of the screen colors
 * temporarily
 */
static void __payload_invert_colors() {
  ESP_LOGD(TAG, "%sINVERT COLORS START", LOG_RED);
  drv_ili9225_rgb_mode(true);
  gfx_push_screen_buffer();
  DELAY(CONFIG_BOTNET_INVERT_COLORS_TIME);
  drv_ili9225_rgb_mode(false);
  gfx_push_screen_buffer();
  ESP_LOGD(TAG, "%sINVERT COLORS STOP", LOG_CYAN);
}

/**
 * @brief execute the payload that joins a botnet
 * @param p_packet : pointer to packet structure that we will use to join their
 * botnet
 */
static void __payload_join(ble_advertisement_botnet_t* p_packet) {
  botnet_state_t* p_state = state_botnet_get();
  p_state->botnet_id = *((uint16_t*)p_packet->serial);
  ESP_LOGD(TAG, "%sJoined botnet 0x%04x", LOG_RED, p_state->botnet_id);
}

/**
 * @brief Execute badgelife mimic payload
 */
static void __payload_mimic_badgelife() {
  ESP_LOGD(TAG, "%sBADGELIFE MIMIC START", LOG_RED);
  ble_special_mode_set(special_ble_mode_badgelife_mimic);
  DELAY(CONFIG_BOTNET_AP_RICK_ROLL_TIME);
  ble_special_mode_set(special_ble_mode_none);
  ESP_LOGD(TAG, "%sBADGELIFE MIMIC STOP", LOG_CYAN);
}

static void __payload_no_bling() {
  ESP_LOGD(TAG, "%sNO BLING START", LOG_RED);
  state_hide_bling_set(true);
  ui_menu_main_regenerate();

  DELAY(BOTNET_HIDE_BLING_TIME_MS);
  state_hide_bling_set(false);
  state_save_indicate();
  ui_menu_main_regenerate();
  ESP_LOGD(TAG, "%sNO BLING STOP", LOG_RED);
}

static void __payload_reverse_screen() {
  ESP_LOGD(TAG, "%sREVERSE SCREEN START", LOG_RED);
  drv_ili9225_mirror(true);
  gfx_push_screen_buffer();

  DELAY(BOTNET_REVERSE_SCREEN_TIME_MS);

  drv_ili9225_mirror(false);
  gfx_push_screen_buffer();
  ESP_LOGD(TAG, "%sREVERSE SCREEN STOP", LOG_RED);
}

static void __payload_unlocks_delete() {
  ESP_LOGD(TAG, "%sDeleting unlocks :( :( :(", LOG_RED);
  state_unlock_set(0);
}

static IRAM_ATTR void __payload_task(void* parameters) {
  ble_advertisement_botnet_t packet;
  while (1) {
    // Discard queue so we get immediate response on next bling
    xQueueReceive(m_payload_queue, &packet, 1 / portTICK_PERIOD_MS);

    // Wait for a successful attack to come through, then execute it
    if (xQueueReceive(m_payload_queue, &packet, portMAX_DELAY)) {
      m_payload_stop = false;
      m_current_payload = packet.payload;
      switch (packet.payload) {
        case botnet_payload_ap_d4rkm4tter:
          __payload_ap_d4rkm4tter();
          break;
        case botnet_payload_ap_rick_roll:
          __payload_ap_rick_roll();
          break;
        case botnet_payload_badgelife_mimic:
          __payload_mimic_badgelife();
          break;
        case botnet_payload_bling:
          __payload_bling(packet.data);
          break;
        case botnet_payload_buttons:
          __payload_buttons();
          break;
        case botnet_payload_clean:
          __payload_clean();
          break;
        case botnet_payload_history_poison:
          __payload_history(&packet);
          break;
        case botnet_payload_invert_colors:
          __payload_invert_colors();
          break;
        case botnet_payload_join:
          __payload_join(&packet);
          break;
        case botnet_payload_no_bling:
          __payload_no_bling();
          break;
        case botnet_payload_reverse_screen:
          __payload_reverse_screen();
          break;
        case botnet_payload_unlocks_delete:
          __payload_unlocks_delete();
          break;

        // Control payloads
        case botnet_payload_none:
          break;
        case botnet_payload_reset:
          break;
        case botnet_payload_count:
          break;
      }
    }

    m_current_payload = botnet_payload_none;
    // Limit how often payloads can run
    DELAY(CONFIG_BOTNET_BLING_RATE_LIMIT);
  }
  // Shouldn't get here but just in case
  vTaskDelete(NULL);
}

void botnet_advertiser_handler(ble_advertisement_botnet_t* packet) {
  botnet_state_t* p_state = state_botnet_get();
  //   char pwned_by[BOTNET_PWNED_BY_LENGTH + 1];
  //   snprintf(pwned_by, BOTNET_PWNED_BY_LENGTH + 1, "%s", packet->pwned_by);
  //	ESP_LOGD(TAG, "%s: cmd=0x%02x data=0x%02x pwned_by=%s, timestamp=%d,
  // ttl=%d", __func__, packet->payload, packet->data, pwned_by,
  // packet->timestamp, packet->ttl);

  // Only queue packets that are considered valid
  if (botnet_payload_validate(packet)) {
    // Parse strength values
    uint16_t attack = packet->strength;
    uint16_t defense = m_botnet_packet.strength;
    // Parse timestamps
    uint32_t attack_ts = *(uint32_t*)packet->timestamp;
    uint32_t attack_random = util_random(0, attack + defense);
    bool same_botnet = (*(uint16_t*)packet->serial == p_state->botnet_id);
    bool is_mean =
        (botnet_payload_types[packet->payload] == botnet_payload_type_mean);
    bool is_nice =
        (botnet_payload_types[packet->payload] == botnet_payload_type_nice);

    ESP_LOGD(TAG,
             "%sValid botnet packet. %s ATT: %d ATT.TS: %d DEF: %d DEF.TS: %d "
             "%sATT.RND: %d",
             LOG_GREEN, LOG_MAGENTA, attack, attack_ts, defense, m_timestamp,
             LOG_YELLOW, attack_random);

    // Stop now if attack is in the past
    if (attack_ts <= m_timestamp) {
      return;
    }

    // Special processing of Master Botnet C2 Commands
    if (packet->payload == botnet_payload_reset) {
      ESP_LOGD(TAG, "%sMASTER RESET!", LOG_BLUE);

      // If bling is being run, stop it
      if (m_current_payload == botnet_payload_bling) {
        gfx_stop();
      }

      // Flag to any running payloads to stop
      m_payload_stop = true;

      m_botnet_packet.payload = botnet_payload_reset;
      m_botnet_packet.data = 0;
      m_botnet_packet.strength = attack - BOTNET_STRENGHT_DEGRADE_PER_HOP;
      *m_botnet_packet.timestamp = *(uint32_t*)packet->timestamp;
      m_timestamp = *(uint32_t*)packet->timestamp;
      return;
    }

    // Attack is successful if Attack > Defense. This limits the spread of
    // payloads regardless of mean or nice
    if (attack_random > defense) {
      bool run_payload = false;
      bool infect = false;
      bool decrement = false;

      // Mean attacks in same botnets only infect but do not run or decrement
      if (is_mean && same_botnet) {
        infect = true;
      }
      // Mean attacks only decrement points and run payloads if opposing
      // botnet
      else if (is_mean && !same_botnet) {
        decrement = true;
        run_payload = true;
        infect = true;
      }
      // Nice attacks run, decrement points, and infect only in same botnet
      else if (is_nice && same_botnet) {
        run_payload = true;
        infect = true;
        decrement = true;
      }

      // Only decrement points in certain instances
      if (decrement) {
        ESP_LOGD(TAG, "%sDecrementing Points", LOG_BLUE);
        if (packet->strength < BOTNET_STRENGHT_DEGRADE_PER_HOP) {
          packet->strength = 0;
        } else {
          packet->strength -= BOTNET_STRENGHT_DEGRADE_PER_HOP;
        }
      }

      // Only infect in certain instances
      if (infect) {
        ESP_LOGD(TAG, "%sInfecting", LOG_BLUE);
        botnet_packet_set(packet);
      }

      // Only run the payload in certain instances
      if (run_payload) {
        ESP_LOGD(TAG, "%sQueuing payload", LOG_BLUE);
        xQueueSend(m_payload_queue, packet, 1 / portTICK_PERIOD_MS);
      }
    }

    // Regardless of attack success, remember the timestamp so we don't keep
    // re-evaluating the same attack
    m_timestamp = attack_ts;
  } else {
    ESP_LOGD(TAG, "%sInvalid botnet packet received", LOG_RED);
  }
}

ble_advertisement_botnet_t* botnet_packet_get() {
#ifdef CONFIG_BADGE_TYPE_STANDALONE
  // In standalone mode generate a random botnet packet
  botnet_state_t* p_state = state_botnet_get();
  m_botnet_packet.payload = botnet_payload_none;
  m_botnet_packet.data = util_random(0, botnet_bling_count);
  m_botnet_packet.strength = util_random(0, 100);
  *(uint16_t*)m_botnet_packet.timestamp = time_manager_now_sec();
  uint16_t crc =
      crc16_le(0, &m_botnet_packet, sizeof(ble_advertisement_botnet_t) - 2);
  memcpy(m_botnet_packet.crc, &crc, sizeof(uint16_t));
  ESP_LOGD(TAG, "Standalone CRC set to 0x%02x", crc);
#endif
  return &m_botnet_packet;
}

/**
 * @brief Initialize the botnet
 */
void botnet_init() {
  m_payload_queue = xQueueCreate(1, sizeof(ble_advertisement_botnet_t));
  static StaticTask_t botnet;

  xTaskCreate(__payload_task, "Botnet Payloads", 6000, NULL,
              TASK_PRIORITY_MEDIUM, NULL);
  util_task_create(__botnet_task, "Botnet", 8192, NULL, TASK_PRIORITY_LOW,
                   &botnet);

  *(uint16_t*)m_botnet_packet.crc = crc16_le(
      0, (uint8_t*)&m_botnet_packet, sizeof(ble_advertisement_botnet_t) - 2);
}

/**
 * @brief Level up the player
 */
void botnet_level_up() {
  botnet_state_t* p_state = state_botnet_get();
  p_state->level = (p_state->level + 1) % BOTNET_LEVEL_MAX;
  ESP_LOGD(TAG, "Botnet level up, new level %d", p_state->level);
}

/**
 * @brief : Set a botnet packet. This includes setting the CRC
 * @param p_packet : Pointer to botnet packet to set
 */
void botnet_packet_set(ble_advertisement_botnet_t* p_packet) {
  memcpy(&m_botnet_packet, p_packet, sizeof(ble_advertisement_botnet_t));
  *(uint16_t*)m_botnet_packet.crc =
      crc16_le(0, (uint8_t*)p_packet, sizeof(ble_advertisement_botnet_t) - 2);
  m_timestamp = *(uint32_t*)(p_packet->timestamp);
  ESP_LOGD(TAG,
           "Setting botnet packet. CRC=0x%02x Timestamp=%d Current Time=%d",
           (uint16_t)*m_botnet_packet.crc, m_timestamp, time_manager_now_sec());
}

/**
 * @brief Validate a given botnet packet
 * @param p_packet : Pointer to the packet to validate
 * @return True if valid
 */
bool botnet_payload_validate(ble_advertisement_botnet_t* p_packet) {
  uint16_t crc =
      crc16_le(0, (uint8_t*)p_packet, sizeof(ble_advertisement_botnet_t) - 2);
  uint16_t packet_crc = *(uint16_t*)p_packet->crc;

  return (packet_crc == crc);
}

/**
 * @brief Reset botnet state, careful player will lose all progress!
 */
void botnet_reset() {
  ESP_LOGD(TAG, "%s", __func__);
  botnet_state_t* p_state = state_botnet_get();
  p_state->level = BOTNET_DEFAULT_LEVEL;
  p_state->points = BOTNET_DEFAULT_POINTS;
  p_state->botnet_id = util_serial_get() & 0xffff;
  p_state->experience = BOTNET_DEFAULT_EXPERIENCE;
  p_state->first_run = true;
}
