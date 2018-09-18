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
#define MARGIN 5
#define COLOR_TITLE COLOR_WESTERN_YELLOW
#define COLOR_TEXT COLOR_WESTERN_ORANGE
#define SCREEN_REFRESH_TIME_MS (60 * 1000)

const static char* TAG = "MRMEESEEKS::Botnet-UI";

static int8_t __pick_bling();
static uint8_t __research_screen(uint8_t cost);

static inline void __draw_buttons() {
  // Help button
  gfx_ui_draw_left_button("<--");

  // Attack button
  gfx_ui_draw_right_button("-->");
}

#ifdef CONFIG_BADGE_TYPE_MASTER
static void __build_master_attack(botnet_payload_t payload) {
  // Craft a botnet packet
  ble_advertisement_botnet_t packet;
  packet.data = 0;
  packet.payload = payload;

  packet.strength = BOTNET_MAX_STRENGTH;
  *(uint16_t*)packet.serial = (uint16_t)util_serial_get();
  *(uint32_t*)packet.timestamp = time_manager_now_sec();

  // Ask the master
  char confirm[128];
  sprintf(confirm, "Run payload '%s'?", botnet_payload_str[payload]);
  if (ui_confirm("Master Payload", confirm) == 1) {
    // Fire!
    botnet_packet_set(&packet);

    char buffer[128];
    sprintf(buffer, "%s mode set", botnet_payload_str[payload]);
    ui_popup_info(buffer);
  }
}
#endif

/**
 * @brief Craft an attack based on the screen indexx
 * @param p_screen_index : pointer to current screen index
 */
static void __build_attack(botnet_screen_t* p_screen_index) {
  botnet_state_t* p_state = state_botnet_get();
  uint8_t cost = BOTNET_RESEARCH_COST;

  // Clean has its own cost
  if (*p_screen_index == botnet_screen_clean) {
    cost = BOTNET_CLEAN_COST;
  }

  // Make sure they have enough points
  if (p_state->points < cost) {
    char buffer[128];
    sprintf(buffer, "Not enough points available! %d required.", cost);
    ui_popup_error(buffer);
    return;
  }

  // Debit the points
  ESP_LOGD(TAG, "Debitting %d points", cost);
  p_state->points -= cost;

  // Craft a botnet packet
  ble_advertisement_botnet_t packet;
  packet.data = 0;
  packet.strength = __research_screen(cost);

  // Strength is 0 if they didn't research
  if (packet.strength > 0) {
    // Override current botnet with user-selected if strong enough
    if (packet.strength >= botnet_packet_get()->strength) {
      // Assign packet payload based on screen selected. Using a switch and enum
      // so compiler can identify some common logic issues
      switch (*p_screen_index) {
        case botnet_screen_bling:
          packet.payload = botnet_payload_bling;
          int8_t bling_picked = __pick_bling();
          packet.data = bling_picked;
          // Quit if they didn't pick a bling mode
          if (bling_picked < 0) {
            return;
          }
          break;
        case botnet_screen_buttons:
          packet.payload = botnet_payload_buttons;
          break;
        case botnet_screen_clean:
          packet.payload = botnet_payload_clean;
          break;
        case botnet_screen_history_poison:
          packet.payload = botnet_payload_history_poison;
          break;
        case botnet_screen_invert_colors:
          packet.payload = botnet_payload_invert_colors;
          break;
        case botnet_screen_join:
          packet.payload = botnet_payload_join;
          break;
        case botnet_screen_no_bling:
          packet.payload = botnet_payload_no_bling;
          break;
        case botnet_screen_reverse_screen:
          packet.payload = botnet_payload_reverse_screen;
          break;
        case botnet_screen_unlocks_delete:
          packet.payload = botnet_payload_unlocks_delete;
          break;

        // Quit botnet cannot spread
        case botnet_screen_quit:
          break;
        // Help screen can't spread
        case botnet_screen_help:
          break;

          // These cases should never happen here, master payloads should not be
          // accessible by mere mortals
#ifdef CONFIG_BADGE_TYPE_MASTER
        case botnet_screen_ap_d4rkm4tter:
        case botnet_screen_ap_rick_roll:
        case botnet_screen_badgelife_mimic:
        case botnet_screen_reset:
#endif
        case botnet_screen_attack:
        case botnet_screen_count:
          break;
      }
      ESP_LOGD(TAG, "Setting packet payload to %d", packet.payload);

      // Current time
      *(uint32_t*)packet.timestamp = time_manager_now_sec();
      *(uint16_t*)packet.serial = (uint16_t)util_serial_get();

      // Fire!
      botnet_packet_set(&packet);

      ui_popup_info("Payload set! Go pwn some n00bs");
    } else {
      char buffer[128];
      sprintf(buffer, "Exploit too weak [%d < %d]. Abort.", packet.strength,
              botnet_packet_get()->strength);
      ui_popup_error(buffer);
    }
  }

  // Tell state something changed
  state_save_indicate();
  *p_screen_index = 0;
}

/**
 * Pick a bling mode by scrolling through severl pre-defined modes
 */
static int8_t __pick_bling() {
  botnet_state_t* p_state = state_botnet_get();
  int8_t bling_mode = 0;
  uint16_t count = botnet_bling_count;
  uint32_t frame = 0;
  uint32_t frames =
      gfx_frame_count(botnet_bling_paths[bling_mode], LCD_WIDTH, LCD_HEIGHT);
  uint16_t w, h;
  int16_t x, y;
  cursor_coord_t cursor;
  gfx_text_bounds(botnet_bling_labels[bling_mode], 0, 0, &x, &y, &w, &h);
  cursor.x = (LCD_WIDTH - w) / 2;
  cursor.y = (LCD_HEIGHT - h) / 2;

  // Limit botnet bling modes by level
  if (p_state->level < 3) {
    count--;
  }
  if (p_state->level < 6) {
    count--;
  }
  if (p_state->level < 10) {
    count--;
  }
  if (p_state->level < 15) {
    count--;
  }

  ESP_LOGD(TAG, "Count = %d botnet_bling_count = %d", count,
           botnet_bling_count);

  while (1) {
    gfx_fill_screen(COLOR_BLACK);
    gfx_draw_raw_frame(botnet_bling_paths[bling_mode], frame, 0, 0, LCD_WIDTH,
                       LCD_HEIGHT, false, 0);

    // Drop shadow
    gfx_font_set(font_large);
    gfx_color_set(COLOR_WHITE);
    gfx_print_drop_shadow(botnet_bling_labels[bling_mode], COLOR_BLACK, cursor);

    __draw_buttons();
    gfx_push_screen_buffer();
    btn_wait_max(50);

    // Change bling mode based on user input
    if (btn_a()) {
      bling_mode--;
      if (bling_mode < 0) {
        bling_mode = count - 1;
      }
    } else if (btn_b()) {
      bling_mode = (bling_mode + 1) % count;
    } else if (btn_c()) {
      return bling_mode;
    } else if (btn_left()) {
      return -1;
    }

    // If any bling mode was changed, recalcuate frames and text coordinates
    if (btn_a() || btn_b()) {
      // reset frame index and count
      frame = 0;
      frames = gfx_frame_count(botnet_bling_paths[bling_mode], LCD_WIDTH,
                               LCD_HEIGHT);

      // Calculate new x coordinate to center text
      x = 0;
      y = 0;
      gfx_text_bounds(botnet_bling_labels[bling_mode], 0, 0, &x, &y, &w, &h);
      cursor.x = (LCD_WIDTH - w) / 2;

      // Slow down the user
      DELAY(70);
    } else {
      frame = (frame + 10) % frames;
    }
  }
}

static void __draw_main_screen() {
  char buffer[256];
  char name[STATE_NAME_LENGTH + 1];
  ble_advertisement_botnet_t* p_botnet_packet = botnet_packet_get();
  botnet_state_t* p_state = state_botnet_get();
  state_name_get(name);

  cursor_coord_t cursor = {MARGIN, 0};

  // used to compute text bounds
  int16_t x, y;
  uint16_t w, h;

  gfx_fill_screen(COLOR_BLACK);
  gfx_draw_raw_file("/sdcard/gfx/botnet-bg.raw", 0, 0, LCD_WIDTH, LCD_HEIGHT,
                    false, 0);
  gfx_font_set(font_large);
  gfx_cursor_set(cursor);
  gfx_color_set(COLOR_TITLE);
  sprintf(buffer, "%s [%02x]", name, p_state->botnet_id);
  gfx_print_drop_shadow(buffer, COLOR_BLACK, cursor);
  cursor.y += gfx_font_height() + 10;

  // Print level
  gfx_font_set(font_medium);
  sprintf(buffer, "Level: %d", p_state->level);
  gfx_text_bounds(buffer, 0, 0, &x, &y, &w, &h);
  cursor.x = LCD_WIDTH - w - MARGIN;
  gfx_cursor_set(cursor);
  gfx_color_set(COLOR_TEXT);
  gfx_print_drop_shadow(buffer, COLOR_BLACK, cursor);
  cursor.y += gfx_font_height() + 2;

  // Print points
  sprintf(buffer, "Points: %d", p_state->points);
  gfx_text_bounds(buffer, 0, 0, &x, &y, &w, &h);
  cursor.x = LCD_WIDTH - w - MARGIN;
  gfx_cursor_set(cursor);
  gfx_print_drop_shadow(buffer, COLOR_BLACK, cursor);
  cursor.y += gfx_font_height() + 2;

  // Print XP
  cursor.x = MARGIN;
  gfx_print_drop_shadow("XP", COLOR_BLACK, cursor);
  cursor.y += gfx_font_height() + 2;
  float pct = (float)p_state->experience /
              (float)(p_state->level * BOTNET_SP_PER_LEVEL_REQD);
  w = (uint16_t)((float)(LCD_WIDTH - (2 * MARGIN)) * pct);
  gfx_fill_rect(MARGIN, cursor.y, w, 16, COLOR_GREEN);
  gfx_draw_rect(MARGIN, cursor.y, LCD_WIDTH - (2 * MARGIN), 16, COLOR_WHITE);
  cursor.y += 16 + 5;

  // Print Payload
  sprintf(buffer, "Payload: %s", botnet_payload_str[p_botnet_packet->payload]);
  gfx_text_bounds(buffer, 0, 0, &x, &y, &w, &h);
  cursor.x = LCD_WIDTH - w - MARGIN;
  gfx_cursor_set(cursor);
  gfx_print_drop_shadow(buffer, COLOR_BLACK, cursor);
  cursor.y += gfx_font_height() + 2;

  // Print Strength
  sprintf(buffer, "Strength: %d", p_botnet_packet->strength);
  gfx_text_bounds(buffer, 0, 0, &x, &y, &w, &h);
  cursor.x = LCD_WIDTH - w - MARGIN;
  gfx_cursor_set(cursor);
  gfx_print_drop_shadow(buffer, COLOR_BLACK, cursor);

  __draw_buttons();

  gfx_push_screen_buffer();
}

/**
 * @brief internal helper function to render a screen for a specific
 * attack/payload
 */
static void __draw_attack_screen(botnet_payload_t payload) {
  uint16_t w, h;
  gfx_fill_screen(COLOR_DARKRED);
  gfx_draw_raw_file(botnet_payload_bg[payload], 0, 0, LCD_WIDTH, LCD_HEIGHT,
                    false, 0);
  gfx_color_set(COLOR_WHITE);
  gfx_font_set(font_large);

  cursor_coord_t cursor = {.x = 0, .y = 0};
  gfx_cursor_set(cursor);
  gfx_print_drop_shadow("Set Payload", COLOR_BLACK, cursor);

  gfx_text_bounds(botnet_payload_str[payload], 0, 0, &cursor.x, &cursor.y, &w,
                  &h);

  // Print attack name
  gfx_color_set(COLOR_WHITE);
  cursor.x += (LCD_WIDTH - w) / 2;
  cursor.y += (LCD_HEIGHT - h) / 2;
  gfx_cursor_set(cursor);
  gfx_print_drop_shadow(botnet_payload_str[payload], COLOR_BLACK, cursor);

  __draw_buttons();
  gfx_push_screen_buffer();
}

/**
 * @brief Draw a menu screen for help
 */
static void __draw_help_screen() {
  cursor_coord_t cursor = {0, 0};
  uint16_t w, h;
  gfx_fill_screen(COLOR_DARKRED);
  gfx_draw_raw_file(botnet_payload_bg[botnet_payload_help], 0, 0, LCD_WIDTH,
                    LCD_HEIGHT, false, 0);
  gfx_color_set(COLOR_WHITE);
  gfx_font_set(font_large);

  char* text = "Help";
  gfx_text_bounds(text, 0, 0, &cursor.x, &cursor.y, &w, &h);

  // Print the label
  gfx_color_set(COLOR_WHITE);
  cursor.x += (LCD_WIDTH - w) / 2;
  cursor.y += (LCD_HEIGHT - h) / 2;
  gfx_cursor_set(cursor);
  gfx_print_drop_shadow(text, COLOR_BLACK, cursor);

  __draw_buttons();
  gfx_push_screen_buffer();
}

/**
 * @brief Render a screen to confirm they want to quit the botnet
 */
static void __do_quit(botnet_screen_t* p_screen_index) {
  botnet_state_t* p_state = state_botnet_get();
  uint8_t cost = p_state->points / 2;

  char quit[128];
  sprintf(quit, "Leave botnet %04X", p_state->botnet_id);
  char cost_str[32];
  sprintf(cost_str, "%d Points", cost);
  if (ui_confirm(quit, cost_str) == 1) {
    p_state->points -= cost;
    p_state->botnet_id = util_serial_get() & 0xffff;
  }

  *p_screen_index = 0;
}

/**
 * @brief Handle research for crafting a botnet attack.
 *
 * 1) Randomly generate an exploit
 * 2) Draw the screen with some faked console stuff
 * 3) Do "research"
 * 4) Wait for user
 *
 * @return Exploit strength
 */
static uint8_t __do_research() {
  botnet_state_t* p_state = state_botnet_get();

  // Pick and exploit
  uint8_t exploit_class = util_random(0, 100);
  uint16_t exploit_strength = 0;

  // Legendary exploit
  if (exploit_class >= 85) {
    exploit_strength = util_random(90, 100);
  }
  // Special exploit
  else if (exploit_class >= 60) {
    exploit_strength = util_random(25, 60);
  }
  // Normal exploit
  else {
    exploit_strength = util_random(10, 25);
  }

  // add in player's level
  exploit_strength += (p_state->level * BOTNET_STRENGTH_PER_LEVEL);

  // Ensure strength doesn't go above a certain amount
  exploit_strength = MIN(exploit_strength, BOTNET_MAX_STRENGTH);

  // Reset the screen
  gfx_fill_screen(COLOR_BLACK);
  gfx_font_set(font_small);
  gfx_color_set(COLOR_GREEN);
  gfx_cursor_set((cursor_coord_t){0, 0});

  char player_level[32];
  sprintf(player_level, "Player level: %d\n", p_state->level);
  char player_points[32];
  sprintf(player_points, "Player points: %d\n", p_state->points);
  char exploit[64];
  sprintf(exploit, "Exploit researched:\n  %d strength", exploit_strength);

  char* text[] = {"AND!XOR b0tneT 2.0\n",
                  "~~~~~~~~~~~~~~~~~~~~~~~~~~~\n",
                  "loading b0tneT modules\n",
                  "  TCP..........",
                  "Loaded\n",
                  "  UDP..........",
                  "Loaded\n",
                  "  HTTP.........",
                  "Loaded\n",
                  "  IoT..........",
                  "Loaded\n",
                  "  EMAIL........",
                  "Loaded\n",
                  "  APT.........."
                  "Loaded\n",
                  "  Bling........",
                  "Loaded\n",
                  "  Cmd & Ctrl...",
                  "Loaded\n\n",
                  "\n",
                  player_level,
                  player_points,
                  "\n",
                  "Researching Exploit\n",
                  "..",
                  "--",
                  "..",
                  "--"
                  "..",
                  "--"
                  "..",
                  "--"
                  "..",
                  "--"
                  "..\n",
                  exploit};
  uint8_t text_count = sizeof(text) / sizeof(uint32_t);

  // Start doing some "research"
  for (uint8_t i = 0; i < text_count; i++) {
    gfx_print(text[i]);
    DELAY(100);
    gfx_push_screen_buffer();
  }

  // Legendary exploit
  if (exploit_class >= 85) {
    gfx_color_set(COLOR_GOLD);
    gfx_print(" LEGENDARY");
    gfx_push_screen_buffer();
  }
  // Special exploit
  else if (exploit_class >= 60) {
    gfx_color_set(COLOR_BLUE);
    gfx_print(" Special");
    gfx_push_screen_buffer();
  }

  btn_wait();
  btn_clear();

  return (uint8_t)exploit_strength;
}

/**
 * Help AKA Mansplain screen
 */
static void __mansplain_screen() {
  cursor_coord_t cursor = {0, 0};
  btn_clear();

  while (1) {
    gfx_fill_screen(COLOR_BLACK);
    gfx_color_set(COLOR_GREEN);
    gfx_font_set(font_large);
    gfx_cursor_set(cursor);
    gfx_print("Mansplaining\n        Botnet 2.0\n");
    gfx_font_set(font_small);
    gfx_print("Things are a bit different this year\n\n");
    gfx_print("This time you don't get to attack your friends\ndirectly.\n\n");
    gfx_print(
        "Now you must team up to\nsilently build a botnet for maximum "
        "rick-roll-age.\n\n");
    gfx_print(
        "It's simple, select a\npayload, rearch, and\ndeploy. Vulnerable "
        "badges\nwill do your bidding\n\n");
    gfx_print(
        "Use 'Join' to build your\nbotnet and push payloads\nfurther\n\n");
    gfx_print("Your botnet ID is available on the main screen.\n\n");
    gfx_print("Gain XP by being near other nodes in your botnet.\n\n");
    gfx_push_screen_buffer();
    btn_wait();

    if (btn_down()) {
      cursor.y -= 10;
    } else if (btn_up()) {
      if (cursor.y < 0) {
        cursor.y += 10;
      } else {
        cursor.y = 0;
      }
    } else if (btn_left() || btn_a() || btn_b() || btn_c()) {
      btn_clear();
      break;
    }
  }
}

/**
 * @brief Mansplain botnet 2.0
 */
static void __first_run() {
  botnet_state_t* p_state = state_botnet_get();
  if (!p_state->first_run) {
    return;
  }

  __mansplain_screen();
  p_state->first_run = false;
}

/**
 * @brief Draw and handle user input for researching an exploit to craft and
 * attack
 */
static uint8_t __research_screen(uint8_t cost) {
  uint16_t w, h;

  gfx_fill_screen(COLOR_YELLOW);
  gfx_draw_raw_file("/sdcard/gfx/botnet-research.raw", 0, 0, LCD_WIDTH,
                    LCD_HEIGHT, false, 0);
  gfx_color_set(COLOR_WHITE);
  gfx_font_set(font_large);

  char* research = "Research";
  cursor_coord_t cursor = {.x = 0, .y = 0};
  gfx_print_drop_shadow(research, COLOR_BLACK, cursor);

  char cost_str[32];
  sprintf(cost_str, "%d Points", cost);
  // Center text
  gfx_text_bounds(cost_str, 0, 0, &cursor.x, &cursor.y, &w, &h);
  cursor.x += (LCD_WIDTH - w) / 2;
  cursor.y += (LCD_HEIGHT - h) / 2;
  gfx_print_drop_shadow(cost_str, COLOR_BLACK, cursor);

  gfx_ui_draw_ok_cancel();
  gfx_push_screen_buffer();

  btn_clear();
  btn_wait();

  if (btn_b() || btn_c()) {
    btn_clear();
    uint8_t exploit_strength = __do_research();
    ESP_LOGD(TAG, "%s exploit strength = %d", __func__, exploit_strength);
    return exploit_strength;
  }

  // Anything else: quit
  btn_clear();

  return 0;
}

void botnet_ui_main() {
  __first_run();

  __attribute((unused)) botnet_state_t* p_state = state_botnet_get();
  botnet_screen_t screen_index = 0;  // current screen
  uint8_t count = botnet_screen_count;

#ifndef CONFIG_BADGE_TYPE_MASTER
  // Hide payloads for not achieving things
  if ((state_unlock_get() & UNLOCK_BANDANA) == 0) {
    count--;
  }
  if ((state_unlock_get() & UNLOCK_LULZCODE) == 0) {
    count--;
  }
  if (p_state->level < 20) {
    count--;
  }
  if (p_state->level < 15) {
    count--;
  }
  if (p_state->level < 10) {
    count--;
  }
  if (p_state->level < 5) {
    count--;
  }
#endif

  while (true) {
    switch (screen_index) {
      case botnet_screen_attack:
        __draw_main_screen();
        break;
      case botnet_screen_bling:
        __draw_attack_screen(botnet_payload_bling);
        break;
      case botnet_screen_buttons:
        __draw_attack_screen(botnet_payload_buttons);
        break;
      case botnet_screen_clean:
        __draw_attack_screen(botnet_payload_clean);
        break;
      case botnet_screen_help:
        __draw_help_screen();
        break;
      case botnet_screen_history_poison:
        __draw_attack_screen(botnet_payload_history_poison);
        break;
      case botnet_screen_join:
        __draw_attack_screen(botnet_payload_join);
        break;
      case botnet_screen_invert_colors:
        __draw_attack_screen(botnet_payload_invert_colors);
        break;
      case botnet_screen_no_bling:
        __draw_attack_screen(botnet_payload_no_bling);
        break;
      case botnet_screen_quit:
        __draw_attack_screen(botnet_payload_quit);
        break;
      case botnet_screen_reverse_screen:
        __draw_attack_screen(botnet_payload_reverse_screen);
        break;
      case botnet_screen_unlocks_delete:
        __draw_attack_screen(botnet_payload_unlocks_delete);
        break;

// Master payloads only accessible by a master badge
#ifdef CONFIG_BADGE_TYPE_MASTER
      case botnet_screen_ap_rick_roll:
        __draw_attack_screen(botnet_payload_ap_rick_roll);
        break;
      case botnet_screen_ap_d4rkm4tter:
        __draw_attack_screen(botnet_payload_ap_d4rkm4tter);
        break;
      case botnet_screen_badgelife_mimic:
        __draw_attack_screen(botnet_payload_badgelife_mimic);
        break;
      case botnet_screen_reset:
        __draw_attack_screen(botnet_payload_reset);
        break;
#endif
      // We don't expect this to happen
      case botnet_screen_count:
        break;
    }

    btn_wait_max(60000);

    if (btn_a()) {
      // Go left
      if (screen_index == 0) {
        screen_index = count - 1;
      } else {
        screen_index--;
      }
      DELAY(80);
    } else if (btn_b()) {
      // Go Right
      screen_index = (screen_index + 1) % count;
      DELAY(80);
    }
    // Handle action button being pressed by executing some behavior based on
    // the screen
    else if (btn_c()) {
      switch (screen_index) {
        case botnet_screen_attack:
          // Do nothing
          break;
        case botnet_screen_bling:
        case botnet_screen_buttons:
        case botnet_screen_clean:
        case botnet_screen_history_poison:
        case botnet_screen_join:
        case botnet_screen_invert_colors:
        case botnet_screen_no_bling:
        case botnet_screen_reverse_screen:
        case botnet_screen_unlocks_delete:
          __build_attack(&screen_index);
          break;
        case botnet_screen_help:
          __mansplain_screen();
          break;
        case botnet_screen_quit:
          __do_quit(&screen_index);
          break;

// Master payloads only accessible by a master badge
#ifdef CONFIG_BADGE_TYPE_MASTER
        case botnet_screen_badgelife_mimic:
          __build_master_attack(botnet_payload_badgelife_mimic);
          break;
        case botnet_screen_reset:
          __build_master_attack(botnet_payload_reset);
          break;
        case botnet_screen_ap_rick_roll:
          __build_master_attack(botnet_payload_ap_rick_roll);
          break;
        case botnet_screen_ap_d4rkm4tter:
          __build_master_attack(botnet_payload_ap_d4rkm4tter);
          break;
#endif
        // We don't expect this to happen
        case botnet_screen_count:
          break;
      }
    } else if (btn_left()) {
      ESP_LOGD(TAG, "bye!");
      return;
    }
  }
}
