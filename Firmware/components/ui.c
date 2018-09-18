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
#define UI_MENU_TITLE_BAR_COLOR COLOR_WESTERN_ORANGE
#define UI_MENU_TITLE_BAR_HEIGHT 10
#define UI_MENU_ITEMS_VISIBLE 5
#define UI_MENU_SCROLL_SPEED 90
#define UI_MENU_BG_COLOR COLOR_BLACK
#define UI_MENU_TEXT_COLOR COLOR_WHITE
// #define UI_MENU_TEXT_COLOR_SELECTED COLOR_WHITE

const static char* TAG = "MRMEESEEKS::UI";

static volatile bool m_allow_interrupt = true;

static volatile bool m_menu_main_regenerate = false;

static void __draw_title_bar();

static inline void __screen_buffer_byte_swap(color_565_t* buffer, size_t size) {
  for (size_t i = 0; i < size; i++) {
    buffer[i] = UINT16_T_SWAP(buffer[i]);
  }
}

static void __menu_render_screen(ui_menu_item_t* p_item,
                                 color_565_t* buffer,
                                 color_565_t* back_buffer) {
  cursor_coord_t cursor = (cursor_coord_t){0, 0};
  int16_t x, y;
  uint16_t w, h;

  // Store current screen, we will be overwriting this then restoring it
  gfx_screen_buffer_copy(back_buffer);

  // Ensure background is at least something
  gfx_fill_screen(UI_MENU_BG_COLOR);
  // Draw background image
  if (p_item->preview != NULL) {
    gfx_draw_raw_file(p_item->preview, 0, 0, LCD_WIDTH, LCD_HEIGHT, false,
                      COLOR_BLACK);
  }

  char* label = p_item->text;

  // Setup cursor and font
  gfx_font_set(font_large);
  gfx_text_bounds(label, 0, 0, &x, &y, &w, &h);
  cursor.x = (LCD_WIDTH - w) / 2;
  cursor.y = (LCD_HEIGHT - UI_MENU_TITLE_BAR_HEIGHT - gfx_font_height()) / 2;
  gfx_cursor_set(cursor);
  gfx_color_set(UI_MENU_TEXT_COLOR);
  // gfx_print(label);
  gfx_print_drop_shadow(label, COLOR_BLACK, cursor);

  // duh
  __draw_title_bar();

  gfx_ui_draw_left_button("<--");
  gfx_ui_draw_right_button("-->");

  gfx_screen_buffer_copy(buffer);
  gfx_screen_buffer_restore(back_buffer);
}

static void __menu_scroll_left(ui_menu_t* p_menu,
                               color_565_t* buffer,
                               color_565_t* back_buffer) {
  if (p_menu->selected > 0) {
    p_menu->selected--;
  } else {
    p_menu->selected = p_menu->count - 1;
  }

  __menu_render_screen(&p_menu->items[p_menu->selected], buffer, back_buffer);
  gfx_screen_buffer_copy(back_buffer);
  __screen_buffer_byte_swap(buffer, LCD_PIXEL_COUNT);
  __screen_buffer_byte_swap(back_buffer, LCD_PIXEL_COUNT);
  for (int16_t x = 0; x <= LCD_WIDTH; x += UI_MENU_SCROLL_SPEED) {
    gfx_draw_raw(x, 0, LCD_WIDTH, LCD_HEIGHT, back_buffer);
    gfx_draw_raw(x - LCD_WIDTH, 0, LCD_WIDTH, LCD_HEIGHT, buffer);
    gfx_push_screen_buffer();
  }
  gfx_draw_raw(0, 0, LCD_WIDTH, LCD_HEIGHT, buffer);
  gfx_push_screen_buffer();
}

static void __menu_scroll_right(ui_menu_t* p_menu,
                                color_565_t* buffer,
                                color_565_t* back_buffer) {
  p_menu->selected++;
  if (p_menu->selected >= p_menu->count) {
    p_menu->selected = 0;
  }

  __menu_render_screen(&p_menu->items[p_menu->selected], buffer, back_buffer);
  gfx_screen_buffer_copy(back_buffer);
  __screen_buffer_byte_swap(buffer, LCD_PIXEL_COUNT);
  __screen_buffer_byte_swap(back_buffer, LCD_PIXEL_COUNT);
  for (int16_t x = 0; x >= (0 - LCD_WIDTH); x -= UI_MENU_SCROLL_SPEED) {
    gfx_draw_raw(x, 0, LCD_WIDTH, LCD_HEIGHT, back_buffer);
    gfx_draw_raw(x + LCD_WIDTH, 0, LCD_WIDTH, LCD_HEIGHT, buffer);
    gfx_push_screen_buffer();
  }
  gfx_draw_raw(0, 0, LCD_WIDTH, LCD_HEIGHT, buffer);
  gfx_push_screen_buffer();
}

/**
 * Simply draw a title bar on the screen
 */
static void __draw_title_bar() {
  cursor_coord_t cursor = (cursor_coord_t){0, 1};
  gfx_font_set(font_small);
  gfx_cursor_set(cursor);
  gfx_color_set(UI_MENU_TITLE_BAR_COLOR);

  // Get botnet info
  botnet_state_t* p_botnet_state = state_botnet_get();

  // Print the name
  char name[STATE_NAME_LENGTH + 1];
  state_name_get(name);

  // Format and print the title bar text
  char text[80];
  sprintf(text, "%s[%04X] - L%d - %2.1fv", name, p_botnet_state->botnet_id,
          p_botnet_state->level, battery_voltage());
  gfx_print(text);

  gfx_draw_line(0, UI_MENU_TITLE_BAR_HEIGHT, LCD_WIDTH,
                UI_MENU_TITLE_BAR_HEIGHT, UI_MENU_TITLE_BAR_COLOR);
}

static void __menu_redraw(ui_menu_t* p_menu,
                          color_565_t* buffer,
                          color_565_t* back_buffer) {
  // Generate initial screen
  __menu_render_screen(&p_menu->items[p_menu->selected], buffer, back_buffer);
  __screen_buffer_byte_swap(buffer, LCD_PIXEL_COUNT);
  gfx_draw_raw(0, 0, LCD_WIDTH, LCD_HEIGHT, buffer);
  gfx_push_screen_buffer();
}

void ui_about() {
  char buffer[64];
  gfx_fill_screen(COLOR_BLACK);
  gfx_color_set(COLOR_GREEN);
  gfx_font_set(font_large);
  gfx_cursor_set((cursor_coord_t){0, 0});
  gfx_print("About\n");
  gfx_font_set(font_small);
  gfx_print("AND!XOR DC26\n");
  gfx_print("Firmware:\n   ");

  gfx_color_set(COLOR_LIGHTGREY);
  sprintf(buffer, "%s\n", VERSION);
  gfx_print(buffer);

  gfx_color_set(COLOR_GREEN);
  gfx_print("ESP-IDF:\n   ");

  gfx_color_set(COLOR_LIGHTGREY);
  sprintf(buffer, "%s\n", IDF_VER);
  gfx_print(buffer);

  gfx_push_screen_buffer();
  btn_wait();
}

/**
 * @brief Internal helper function that counts the number of characters in a
 * word
 */
static inline uint16_t __word_length(const char* str) {
  int tempindex = 0;
  while (str[tempindex] != ' ' && str[tempindex] != 0 &&
         str[tempindex] != '\n') {
    ++tempindex;
  }
  return (tempindex);
}

/**
 * @brief Internal helper function that wraps text by inserting newlines
 */
static void __wrap_text(char* s, uint16_t max_width) {
  int index = 0;
  int curlinelen = 0;
  while (s[index] != '\0') {
    if (s[index] == '\n') {
      curlinelen = 0;
    } else if (s[index] == ' ') {
      uint16_t word_len = __word_length(&s[index + 1]);
      if (curlinelen + word_len >= max_width) {
        s[index] = '\n';
        curlinelen = 0;
      }
    }

    curlinelen++;
    index++;
  }
}

/**
 * @brief Set the allow interrupt flag which tells the UI if we can handle being
 * interrupted by ralph (or any other animation)
 * @param allow : Set to true to allow interrupts false if not
 */
void ui_allow_interrupt(bool allow) {
  m_allow_interrupt = allow;
}

/**
 * @brief Draw and handle user input for a menu
 *
 * @param p_menu : pointer to to a menu structure containing everything
 * necessary for a menu
 */
void ui_menu(ui_menu_t* p_menu) {
  color_565_t* buffer = util_heap_alloc_ext(LCD_BUFFER_SIZE);
  color_565_t* back_buffer = util_heap_alloc_ext(LCD_BUFFER_SIZE);

  __menu_redraw(p_menu, buffer, back_buffer);

  // Block user only allowing menu. Pattern is simple, draw the menu then wait
  // for user input. Based on input adjust some values and redraw
  // Also run forever, unless flagged to regenerate
  while (!m_menu_main_regenerate) {
    btn_wait();
    if (btn_b() || btn_right()) {
      __menu_scroll_right(p_menu, buffer, back_buffer);
    } else if (btn_a() || btn_left()) {
      __menu_scroll_left(p_menu, buffer, back_buffer);
    } else if (btn_c()) {
      ESP_LOGD(TAG, "Selected %d", p_menu->selected);
      ui_menu_item_t item = p_menu->items[p_menu->selected];
      if (item.callback != NULL) {
        item.callback(item.data);
        btn_clear();
        __menu_redraw(p_menu, buffer, back_buffer);
      } else {
        break;
      }
    }
  }

  free(buffer);
  free(back_buffer);
}

static void __menu_bling(void* data) {
  if (!state_hide_bling_get()) {
    lolcode_execute("/sdcard/bling.lulz", true);
  }
}

static void __menu_botnet(void* data) {
  botnet_ui_main();
}

/**
 * @brief Handler for when broadcast menu item is selected. Run the right lol
 * file
 */
static void __menu_broadcast(void* data) {
  lolcode_execute("/sdcard/broadcast.lulz", true);
}

static void __menu_games(void* data) {
  lolcode_execute("/sdcard/games.lulz", true);
}

#ifdef CONFIG_BADGE_TYPE_MASTER
/**
 * @brief Cheat menu
 */
static void __menu_cheat() {
  botnet_state_t* p_state = state_botnet_get();
  char* items[] = {"nvm...", "Level Up", "1000 Points", "Clean Botnet",
                   "Unlock All"};
  uint8_t count = 5;
  uint8_t selected = ui_pick_list("cH3@tz", items, count);
  char buffer[128];

  switch (selected) {
    case 0:
      ui_popup_info("MKAY BYE");
      break;
    case 1:
      botnet_level_up();
      state_save_indicate();
      sprintf(buffer, "LEVEL UP! Now: %d", p_state->level);
      ui_popup_info(buffer);
      break;
    case 2:
      p_state->points += 1000;
      state_save_indicate();
      sprintf(buffer, "+1000 points! Now: %d pts", p_state->points);
      ui_popup_info(buffer);
      break;
    case 3:;
      ble_advertisement_botnet_t packet;
      packet.payload = botnet_payload_none;
      *(uint32_t*)packet.timestamp = time_manager_now_sec();
      botnet_packet_set(&packet);
      break;
    case 4:
      state_unlock_set(0xFFFF);
      state_save_indicate();
      ui_menu_main_regenerate();
      ui_popup_info("Next time try harder. All unlocks granted.");
      break;
  }
}
#endif

static void __menu_console(void* data) {
  cursor_coord_t cursor = {0, 0};

  while (1) {
    gfx_fill_screen(COLOR_BLACK);
    gfx_color_set(COLOR_GREEN);
    gfx_font_set(font_large);
    gfx_cursor_set(cursor);
    gfx_print("B.E.N.D.E.R.\n");
    gfx_font_set(font_small);
    gfx_print(
        "Welcome to B.E.N.D.E.R\n"
        "The Badge-Enabled\n"
        "Non-Directive Enigma\n"
        "Routine\n"
        "B.E.N.D.E.R...\n\n"
        "The badge contains a text\n"
        "based adventure game which\n"
        "you access via a console.\n\n"
        "This is your interface to\n"
        "the hacking puzzles.\n"
        "Multidisciplinary hacking\n"
        "skills are required to win.\n\n"
        "If you don't know how to do\n"
        "something, go to the \n"
        "appropriate village, make\n"
        "friends, be humble, and ask\n"
        "if someone can teach you\n"
        "something new. Maybe even\n"
        "hook them up a b33r...\n"
        "Thats what this is all\n"
        "about. Make friends, learn,\n"
        "hack, and dont be a dick!\n"
        "\n"
        "The badge saves your state\n"
        "once every 2 minutes. So\n"
        "wait before disconnecting\n"
        "your badge to ensure\n"
        "status is saved.\n"
        "\n"
        "Contact AND!XOR via Twitter\n"
        "and hyr0n@andnxor.com\n"
        "if you win the challenge...\n"
        "\n"
        "Full details on the console\n"
        "are available at our\n"
        "Hackaday.io project page.\n\n"
        "***********SETUP***********\n\n"
        "1.Install SiLabs CP2102N\n"
        "  USB to UART VCP Drivers.\n"
        "  Linux has native support.\n"
        "  Windows & OSX do not.\n"
        "  Badge SD Card has them.\n"
        "\n"
        "2.Install PuTTY\n"
        "  (or any other terminal\n"
        "  that supports ANSI).\n"
        "\n"
        "3.Configure & Connect\n"
        "   115200 Baud\n"
        "   8 Data Bits\n"
        "   1 Stop Bit\n"
        "   No Parity\n"
        "   No Flow Control\n"
        "   110 Window Columns\n"
        "   40 Window Rows\n"
        "\n");
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
    } else if (btn_left()) {
      btn_clear();
      break;
    }
  }
}

static void __menu_physics(void* data) {
  lolcode_execute("/sdcard/particle.lulz", true);
}

static void __menu_system(void* data) {
  lolcode_execute("/sdcard/settings.lulz", true);
}

static void __menu_unlocks(void* data) {
  lolcode_execute("/sdcard/unlock.lulz", true);
}

/**
 * @brief Use a modal dialog to get confirmation input from the user
 * @param prompt : Main prompt to show
 * @param text : String to print in the dialog
 *
 * @return : 1 if OK, 0 if CANCEL
 */
uint8_t ui_confirm(char* prompt, char* text) {
  uint16_t w, h;

  // Save buffer to restore later
  void* buffer = util_heap_alloc_ext(LCD_BUFFER_SIZE);
  gfx_screen_buffer_copy(buffer);

  gfx_fill_screen(COLOR_DARKBLUE);
  gfx_color_set(COLOR_WHITE);
  gfx_font_set(font_large);

  cursor_coord_t cursor = {.x = 0, .y = 0};
  gfx_print_drop_shadow(prompt, COLOR_BLACK, cursor);

  // Center text
  gfx_text_bounds(text, 0, 0, &cursor.x, &cursor.y, &w, &h);
  cursor.x += (LCD_WIDTH - w) / 2;
  cursor.y += (LCD_HEIGHT - h) / 2;
  gfx_print_drop_shadow(text, COLOR_BLACK, cursor);

  gfx_ui_draw_ok_cancel();

  uint8_t result = 0;

  while (1) {
    gfx_push_screen_buffer();
    btn_clear();
    btn_wait();

    if (btn_b() || btn_c()) {
      result = 1;
      break;
    } else if (btn_a()) {
      result = 0;
      break;
    }
  }

  // Restore screen
  gfx_screen_buffer_restore(buffer);
  free(buffer);

  // Anything else: quit
  btn_clear();

  return result;
}

/**
 * @brief Get input from the user
 * @param title : The prompt for the user
 * @param buffer : Pointer to buffer of starting value to use for input. Gets
 * replaced by user
 * @param length : The max length of the input
 */
void ui_input(char* title, char* buffer, uint8_t length) {
  cursor_coord_t cursor = {0, 0};
  ESP_LOGD(TAG, "%s Length = %d", __func__, length);

  // Ensure length is limited
  length = MIN(length, 14);
  length = MAX(length, 1);

  ESP_LOGD(TAG, "%s Length = %d", __func__, length);

  int16_t position = 0;
  int16_t middle = (LCD_HEIGHT - 10) / 2;

  // Single character char array for printing, kinda hacky
  char temp[2];
  temp[1] = 0;

  // Save screen
  void* screen_buffer = util_heap_alloc_ext(LCD_BUFFER_SIZE);
  gfx_screen_buffer_copy(screen_buffer);

  // Ensure previous input doesn't affect us
  btn_clear();

  while (1) {
    gfx_fill_screen(COLOR_DARKBLUE);
    gfx_color_set(COLOR_WHITE);
    gfx_font_set(font_large);
    cursor.x = 0;
    cursor.y = 0;
    gfx_cursor_set(cursor);

    // Title
    gfx_print(title);

    gfx_font_set(font_medium);

    // Print each column
    for (uint8_t column = 0; column < length; column++) {
      if (buffer[column] < INPUT_CHAR_MIN) {
        buffer[column] = INPUT_CHAR_MIN;
      } else if (buffer[column] > INPUT_CHAR_MAX) {
        buffer[column] = INPUT_CHAR_MAX;
      }

      // Determine character color
      if (position == column) {
        // Preview other characters
        gfx_color_set(COLOR_LIGHTBLUE);
        for (uint8_t row = 1; row <= 3; row++) {
          // Print preview above
          temp[0] = buffer[column] - row;
          cursor.x = column * 15;
          cursor.y = middle - (row * 15);
          gfx_cursor_set(cursor);
          gfx_print(temp);

          // Print preview below
          temp[0] = buffer[column] + row;
          cursor.x = column * 15;
          cursor.y = middle + (row * 15);
          gfx_cursor_set(cursor);
          gfx_print(temp);
        }

        // Middle row should be red for selected character
        gfx_color_set(COLOR_RED);
      } else {
        gfx_color_set(COLOR_WHITE);
      }

      // Print single character
      // Move the cursor
      cursor.x = column * 15;
      cursor.y = middle;
      gfx_cursor_set(cursor);
      temp[0] = buffer[column];
      gfx_print(temp);
    }

    gfx_ui_draw_left_button("PgUp");
    gfx_ui_draw_right_button("PgDown");

    gfx_push_screen_buffer();
    btn_wait();

    if (btn_left()) {
      position--;
      DELAY(200);
    } else if (btn_right()) {
      position++;
      DELAY(200);
    } else if (btn_up()) {
      buffer[position]--;
      DELAY(100);
    } else if (btn_down()) {
      buffer[position]++;
      DELAY(100);
    } else if (btn_a()) {
      buffer[position] -= 5;
    } else if (btn_b()) {
      buffer[position] += 5;
    } else if (btn_c()) {
      break;
    }

    // Ensure position doesn't go out of bounds
    if (position >= length) {
      position = length - 1;
    }
    if (position < 0) {
      position = 0;
    }
  }

  // truncate the string
  for (uint8_t i = strlen(buffer) - 1; i > 0; i--) {
    if (buffer[i] != ' ') {
      break;
    }
    buffer[i] = 0;
  }

  // Restore screen buffer
  gfx_screen_buffer_restore(screen_buffer);
  free(screen_buffer);
}

/**
 * @brief Interrupt the UI to scroll a message and slowly play an animation
 * @param message : Message to scroll
 * @param raw_file : file to play
 * @param duration : Length of time in seconds to interrupt
 */
void ui_interrupt(char* message, char* raw_file, uint32_t duration) {
  // Stop early if interrupts not allowed
  if (!m_allow_interrupt) {
    return;
  }

  // Save the current screen buffer
  void* orig_screen_buffer = util_heap_alloc_ext(LCD_BUFFER_SIZE);
  gfx_screen_buffer_copy(orig_screen_buffer);

  // Setup text coordinates
  font_t orig_font = gfx_font_get();
  gfx_font_set(font_large);
  bool orig_wrap = gfx_font_wrap_get();
  gfx_font_wrap_set(false);
  gfx_color_set(COLOR_WHITE);
  cursor_coord_t cursor;
  cursor.x = LCD_WIDTH;
  cursor.y = (LCD_HEIGHT - gfx_font_height()) / 2;
  uint16_t w = 20 * strlen(message);

  // Get metadata from the raw file
  uint32_t frame = 0;
  uint32_t frames = gfx_frame_count(raw_file, LCD_WIDTH, LCD_HEIGHT);

  // Determine an end time 5 seconds from now
  uint32_t end_time = time_manager_now_sec() + duration;

  gfx_pause(true);
  btn_allow_user_input(false);
  while (time_manager_now_sec() < end_time) {
    gfx_fill_screen(COLOR_BLACK);
    gfx_cursor_set(cursor);

    // Draw current frame of the animation
    gfx_draw_raw_frame(raw_file, frame, 0, 0, LCD_WIDTH, LCD_HEIGHT, false, 0);
    // Print the Hello message
    gfx_print(message);
    // Push!
    gfx_push_screen_buffer();

    // Scroll text to the left
    cursor.x -= 30;

    // Advance animation frame, wrapping around
    frame = (frame + 3) % frames;

    // Wrap cursor around if necessary
    if (cursor.x < (0 - w)) {
      cursor.x = LCD_WIDTH;
    }
  }
  btn_allow_user_input(true);
  gfx_pause(false);

  // Restore settings
  gfx_font_set(orig_font);
  gfx_font_wrap_set(orig_wrap);
  gfx_screen_buffer_restore(orig_screen_buffer);
  free(orig_screen_buffer);
  gfx_push_screen_buffer();
}

/**
 * @brief Main menu
 */
void ui_menu_main(void* parameters) {
  // Allocate some memory on the heap for the main menu items
  ui_menu_t* p_menu = (ui_menu_t*)util_heap_alloc_ext(sizeof(ui_menu_t));
  ui_menu_item_t* p_items =
      (ui_menu_item_t*)util_heap_alloc_ext(sizeof(ui_menu_item_t) * 10);

  p_menu->selected = 0;
  p_menu->top = 0;
  p_menu->count = 0;
  if (!state_hide_bling_get()) {
    p_items[p_menu->count++] = (ui_menu_item_t){
        "Bling!", "/sdcard/bg/and!xor.raw", __menu_bling, NULL};
  }
  p_items[p_menu->count++] = (ui_menu_item_t){
      "Botnet", "/sdcard/gfx/botnet-bg.raw", __menu_botnet, NULL};
  p_items[p_menu->count++] =
      (ui_menu_item_t){"Games", "/sdcard/bg/skifree.raw", __menu_games, NULL};
  p_items[p_menu->count++] = (ui_menu_item_t){
      "B.E.N.D.E.R.", "/sdcard/bg/console.raw", __menu_console, NULL};
  if ((state_unlock_get() & UNLOCK_SKI_FREE) > 0) {
    p_items[p_menu->count++] = (ui_menu_item_t){
        "Broadcast", "/sdcard/bg/broadcast.raw", __menu_broadcast, NULL};
  }
  p_items[p_menu->count++] = (ui_menu_item_t){
      "Physics", "/sdcard/bg/physics.raw", __menu_physics, NULL};
  p_items[p_menu->count++] = (ui_menu_item_t){
      "Unlocks", "/sdcard/bg/unlocks.raw", __menu_unlocks, NULL};
  p_items[p_menu->count++] = (ui_menu_item_t){
      "System", "/sdcard/bg/settings.raw", __menu_system, NULL};
#ifdef CONFIG_BADGE_TYPE_MASTER
  p_items[p_menu->count++] =
      (ui_menu_item_t){"cH3@tz", "/sdcard/bg/and!xor.raw", __menu_cheat, NULL};
#endif

  p_menu->items = p_items;

  m_menu_main_regenerate = false;
  ui_menu(p_menu);

  // cleanup
  free(p_menu);
  free(p_items);
}

/**
 * @brief Tell main menu to regenerate itself
 */
void ui_menu_main_regenerate() {
  m_menu_main_regenerate = true;
}

/**
 * @brief Present a list to the user to pick from
 * @param title : title to present to user
 * @param items : array of strings to pick from
 * @param count : number of strings in the array
 * @return index of the selected item
 */
uint8_t ui_pick_list(char* title, char** items, uint8_t count) {
  cursor_coord_t cursor = {0, 0};
  uint8_t top = 0;
  uint8_t count_per_screen = MIN(count, 8);
  int8_t selected = 0;
  btn_clear();

  while (1) {
    gfx_color_set(COLOR_WHITE);
    gfx_fill_screen(COLOR_DARKBLUE);
    gfx_font_set(font_large);
    cursor.x = 0;
    cursor.y = 0;
    gfx_cursor_set(cursor);
    gfx_print(title);

    gfx_font_set(font_medium);

    for (uint8_t i = 0; i < count_per_screen; i++) {
      // Highlight selected text
      if (selected == top + i) {
        gfx_color_set(COLOR_RED);
      } else {
        gfx_color_set(COLOR_WHITE);
      }

      cursor.x = 0;
      cursor.y = 42 + (i * 15);
      gfx_cursor_set(cursor);
      gfx_print(items[top + i]);
    }

    gfx_push_screen_buffer();

    // Wait for user input
    btn_wait();

    // Scroll down
    if (btn_down()) {
      // Move cursor down
      selected++;
      if (selected >= count) {
        selected = count - 1;
      }
      ESP_LOGD(TAG, "selected=%d top=%d", selected, top);
      // Scroll list
      if (selected - top >= count_per_screen) {
        top = selected - count_per_screen + 1;
      }
      DELAY(200);
    }
    // Scroll up
    else if (btn_up()) {
      // Move cursor up
      if (selected > 0) {
        selected--;
      }
      // Scroll list
      if (selected < top) {
        top = selected;
      }
      DELAY(200);
    }
    // Select an item
    else if (btn_c()) {
      btn_clear();

      return selected;
    }
  }
}

/**
 * @brief Allow user to pick a name or enter their own. Save to badge state
 */
void ui_pick_name() {
  char* names[] = {"Custom",   "Bender", "Fry",     "Zapp",    "Kif", "Leela",
                   "Zoidberg", "Amy",    "Nibbler", "Scruffy", "Mom", "Morbo"};
  uint8_t count = 12;

  // Pick a name
  uint8_t selected = ui_pick_list("Name:", names, count);

  char name[STATE_NAME_LENGTH + 1];
  memset(name, 0, STATE_NAME_LENGTH + 1);
  state_name_get(name);

  // Let them write a name
  if (selected == 0) {
    ui_input("Name", name, STATE_NAME_LENGTH);
  }
  // Canned name
  else {
    sprintf(name, names[selected]);
  }

  char buffer[128];
  sprintf(buffer, "Set name to\n   '%s'?", name);
  if (ui_confirm("Name", buffer)) {
    state_name_set(name);
    state_save_indicate();
  }
}

/**
 * @brief Generate a modal popup to indicate something to the user
 *
 * @param text : Text to display
 * @param fg : Foreground color
 * @param bg: Background color
 */
void ui_popup(char* text, color_565_t fg, color_565_t bg) {
  int16_t margin = 30;

  // Save the current screen
  void* screen_buffer = util_heap_alloc_ext(LCD_BUFFER_SIZE);
  gfx_screen_buffer_copy(screen_buffer);

  ui_textbox(margin, margin, LCD_WIDTH - (margin * 2),
             LCD_HEIGHT - (margin * 2), 0, text, fg, bg);

  gfx_push_screen_buffer();

  DELAY(500);
  btn_clear();
  btn_wait();
  btn_clear();

  gfx_screen_buffer_restore(screen_buffer);
  gfx_push_screen_buffer();
  free(screen_buffer);
}

/**
 * @brief Generate an info modal popup to indicate something to the user
 *
 * @param text : Text to display
 */
void ui_popup_info(char* text) {
  ui_popup(text, COLOR_WHITE, COLOR_DARKBLUE);
}

/**
 * @brief Generate an error modal popup to indicate something to the user
 * @param text : Text to display
 */
void ui_popup_error(char* text) {
  ui_popup(text, COLOR_WHITE, COLOR_DARKRED);
}

void ks_backer_credits() {
  gfx_print("!#-b1un7\n");
  gfx_print("!lyaO\n");
  gfx_print("!nfect_@LL\n");
  gfx_print("@afterpacket\n");
  gfx_print("@eenoch\n");
  gfx_print("(|-|\n");
  gfx_print("@add1son7\n");
  gfx_print("@aprilwright\n");
  gfx_print("@badgerops\n");
  gfx_print("@Bininaut\n");
  gfx_print("@d1g1t4l_t3mpl4r\n");
  gfx_print("@Ellwoodthewood\n");
  gfx_print("@ftalligator\n");
  gfx_print("@hackerhiker\n");
  gfx_print("@iampoliteaf\n");
  gfx_print("@indecentsec\n");
  gfx_print("@kjake\n");
  gfx_print("@kur3us\n");
  gfx_print("@matrosov\n");
  gfx_print("@mcmahoniel\n");
  gfx_print("@mzbat & @theDevilsVoice\n");
  gfx_print("@NullPriestess\n");
  gfx_print("@p0lr_\n");
  gfx_print("@pidooma\n");
  gfx_print("@pierogipowered\n");
  gfx_print("@rj_chap\n");
  gfx_print("@sethlaw\n");
  gfx_print("@techgirlmn\n");
  gfx_print("@yaxisbot\n");
  gfx_print("_whatshisface__\n");
  gfx_print("0x4A454646\n");
  gfx_print("0xEnder\n");
  gfx_print("3mul0r\n");
  gfx_print("3ndG4me\n");
  gfx_print("3xd3l!\n");
  gfx_print("5imple0ne\n");
  gfx_print("800xl\n");
  gfx_print("A\n");
  gfx_print("Aardvark\n");
  gfx_print("Abraxas3d\n");
  gfx_print("AC4\n");
  gfx_print("ahhh\n");
  gfx_print("Ajediday\n");
  gfx_print("algirhythm\n");
  gfx_print("All Glory To The Hypnotoad!\n");
  gfx_print("ALL-FOR-ONE\n");
  gfx_print("Amie D.D.\n");
  gfx_print("Andrew Manley\n");
  gfx_print("Andrew Strutt r0d3nt\n");
  gfx_print("Andy Thompson.\n");
  gfx_print("anotheremily\n");
  gfx_print("Anthony Sasadeusz\n");
  gfx_print("arko\n");
  gfx_print("asiverly@gmail.com\n");
  gfx_print("AvgJoe and Zonii\n");
  gfx_print("B334nerd\n");
  gfx_print("babint\n");
  gfx_print("Bart de Boisblanc\n");
  gfx_print("beardbyte\n");
  gfx_print("Becky Powell\n");
  gfx_print("Belouve\n");
  gfx_print("bewmIES\n");
  gfx_print("Big McLargeHuge\n");
  gfx_print("Billcat\n");
  gfx_print("bit_plumber\n");
  gfx_print("Blu69\n");
  gfx_print("BM\n");
  gfx_print("Brad Welch\n");
  gfx_print("Brandalf the Wise\n");
  gfx_print("Brandon0ST\n");
  gfx_print("Brendan O’Connor\n");
  gfx_print("Brendan stubbs\n");
  gfx_print("C65EC386983CBB83\n");
  gfx_print("CarHackingVillage\n");
  gfx_print("caskey\n");
  gfx_print("cesston\n");
  gfx_print("Chris Gallizzi\n");
  gfx_print("claviger\n");
  gfx_print("Colt\n");
  gfx_print("Cosmo Valtran\n");
  gfx_print("Covfefe xDD\n");
  gfx_print("Cryo\n");
  gfx_print("Crypty McCryptoFace\n");
  gfx_print("cstead\n");
  gfx_print("CyberSulu\n");
  gfx_print("CypherCon is Amazing!\n");
  gfx_print("D.G.S.T.\n");
  gfx_print("D3f@17\n");
  gfx_print("damikey\n");
  gfx_print("darkgrue\n");
  gfx_print("darkmynd\n");
  gfx_print("DCZia // LithoChasm\n");
  gfx_print("de4db4be\n");
  gfx_print("Defcon Scavenger Hunt\n");
  gfx_print("Deren S\n");
  gfx_print("dflo16@gmail.com\n");
  gfx_print("dHoetger\n");
  gfx_print("DigitalTinker\n");
  gfx_print("Dixie Flatline\n");
  gfx_print("djsmegma\n");
  gfx_print("DMack\n");
  gfx_print("Doctor Fuck Face\n");
  gfx_print("Dr. Avril\n");
  gfx_print("dracon1c\n");
  gfx_print("Duck Duck\n");
  gfx_print("dustyfresh\n");
  gfx_print("eduncan911\n");
  gfx_print("Edward Snowden\n");
  gfx_print("effjay\n");
  gfx_print("elektik\n");
  gfx_print("EmberFox\n");
  gfx_print("enderst\n");
  gfx_print("enxire (Alissa)\n");
  gfx_print("ERROR418\n");
  gfx_print("Esteban Rojas V\n");
  gfx_print("Exibar\n");
  gfx_print("Fallible\n");
  gfx_print("Fallon\n");
  gfx_print("Feeding the fun\n");
  gfx_print("Flattire\n");
  gfx_print("Foobar\n");
  gfx_print("FragNet\n");
  gfx_print("FrozenFOXX\n");
  gfx_print("gadams\n");
  gfx_print("GG\n");
  gfx_print("Ghoti\n");
  gfx_print("Grayhell\n");
  gfx_print("Greg Christopher\n");
  gfx_print("groved\n");
  gfx_print("Hacker Warehouse\n");
  gfx_print("Hax0r Bax0r\n");
  gfx_print("Hax4Tats\n");
  gfx_print("HealWHans H4K\n");
  gfx_print("HenrikJay\n");
  gfx_print("Hermit\n");
  gfx_print("heymeitsyou\n");
  gfx_print("Honest Adrian\n");
  gfx_print("hsudoer\n");
  gfx_print("httpLov3craft\n");
  gfx_print("hyp0xic\n");
  gfx_print("Ian Michaels\n");
  gfx_print("iDigitalFlame\n");
  gfx_print("IdiotSavant\n");
  gfx_print("Infosecanon\n");
  gfx_print("IO_Winning\n");
  gfx_print("iqlusion\n");
  gfx_print("IrishMASMS\n");
  gfx_print("ITSec_Brewer\n");
  gfx_print("iukea\n");
  gfx_print("Jark22\n");
  gfx_print("Jay Radcliffe\n");
  gfx_print("Jeff “BigTaro” G.\n");
  gfx_print("Jeremy Hong\n");
  gfx_print("Jeremy Solt\n");
  gfx_print("Jerry Navarro\n");
  gfx_print("Jim Wasson\n");
  gfx_print("Joe Christian\n");
  gfx_print("Jofo\n");
  gfx_print("Johnnie O\n");
  gfx_print("JoSko\n");
  gfx_print("jperez\n");
  gfx_print("jthoel\n");
  gfx_print("Just Jay\n");
  gfx_print("Jutral\n");
  gfx_print("Kevin Colley\n");
  gfx_print("kevvyg\n");
  gfx_print("Kexel\n");
  gfx_print("Khyron\n");
  gfx_print("KPH\n");
  gfx_print("Krux\n");
  gfx_print("KyleD?\n");
  gfx_print("L34N\n");
  gfx_print("lokolyokol\n");
  gfx_print("Lord_Rion\n");
  gfx_print("LSCoolJ\n");
  gfx_print("Lucy\n");
  gfx_print("LV\n");
  gfx_print("M1k3@\n");
  gfx_print("Magicalbeard\n");
  gfx_print("MalMcWil\n");
  gfx_print("Manos\n");
  gfx_print("mathcrab\n");
  gfx_print("matt ploessel\n");
  gfx_print("mauvehed\n");
  gfx_print("me\n");
  gfx_print("Megan Sferrazza\n");
  gfx_print("MikeDan\n");
  gfx_print("mindcrank\n");
  gfx_print("Mr. Bill / @SecureThisNow\n");
  gfx_print("MSFT_BOB!\n");
  gfx_print("Must Be Art\n");
  gfx_print("n0m4d1c\n");
  gfx_print("NB\n");
  gfx_print("neuralbladez\n");
  gfx_print("nick cano\n");
  gfx_print("Nick Miles\n");
  gfx_print("None\n");
  gfx_print("Nope.\n");
  gfx_print("Norman Lundt\n");
  gfx_print("North\n");
  gfx_print("nospam@mostlyincorrect.info\n");
  gfx_print("Nothing\n");
  gfx_print("nou\n");
  gfx_print("Nullifidian\n");
  gfx_print("numinit\n");
  gfx_print("OhYou_\n");
  gfx_print("ohyouknow\n");
  gfx_print("ONE-FOR-ALL\n");
  gfx_print("Oss KC\n");
  gfx_print("ouno\n");
  gfx_print("p0lr\n");
  gfx_print("p1nk\n");
  gfx_print("P373 H\n");
  gfx_print("p414din\n");
  gfx_print("Paleck\n");
  gfx_print("PatAttack Jones\n");
  gfx_print("phreakocious\n");
  gfx_print("pii100\n");
  gfx_print("pizen\n");
  gfx_print("quentin s.\n");
  gfx_print("@r00tkillah\n");
  gfx_print("r3x3r\n");
  gfx_print("Raitlin\n");
  gfx_print("Reality\n");
  gfx_print("Ret0n\n");
  gfx_print("RIP [DC801]D3c4f\n");
  gfx_print("RKUltra\n");
  gfx_print("rrx69\n");
  gfx_print("RTJR\n");
  gfx_print("S1r3nn\n");
  gfx_print("saltr\n");
  gfx_print("SaltySec\n");
  gfx_print("Sausage Mahoney\n");
  gfx_print("schlick\n");
  gfx_print("Scoot\n");
  gfx_print("Scotch and Bubbles\n");
  gfx_print("Scott Phayl Perzan\n");
  gfx_print("SecBarbie\n");
  gfx_print("sec-princess\n");
  gfx_print("shlomithemoney\n");
  gfx_print("ShogoC\n");
  gfx_print("SinderzNAshes\n");
  gfx_print("Skipper Blue\n");
  gfx_print("Skipper Blue again\n");
  gfx_print("Skybye\n");
  gfx_print("Slaugh\n");
  gfx_print("Sloth\n");
  gfx_print("smo0otchy\n");
  gfx_print("Snozzberries\n");
  gfx_print("SodaCannonGuy\n");
  gfx_print("Sonicos\n");
  gfx_print("Sp3nx0r\n");
  gfx_print("SparX\n");
  gfx_print("Spencer\n");
  gfx_print("Spike\n");
  gfx_print("sqearlsalazar\n");
  gfx_print("SriLankanMonkey\n");
  gfx_print("Star-Fox\n");
  gfx_print("Stay Hydrated\n");
  gfx_print("strategicpause\n");
  gfx_print("Sure #badgelife @fg\n");
  gfx_print("Sure why not\n");
  gfx_print("Sutur\n");
  gfx_print("szrachen\n");
  gfx_print("T3h Ub3r K1tten\n");
  gfx_print("Teacup\n");
  gfx_print("Telspace Systems\n");
  gfx_print("THE GAME\n");
  gfx_print("The Hat\n");
  gfx_print("The_Stevus\n");
  gfx_print("thecl0ud\n");
  gfx_print("theTOOLMAN\n");
  gfx_print("Thorage\n");
  gfx_print("ThreeChip\n");
  gfx_print("thule\n");
  gfx_print("Tooms\n");
  gfx_print("Travelar\n");
  gfx_print("TrippBit\n");
  gfx_print("trithemius\n");
  gfx_print("tweek\n");
  gfx_print("TwinkleTwinkie\n");
  gfx_print("UTMafia\n");
  gfx_print("variable.label\n");
  gfx_print("VDALABS\n");
  gfx_print("w00kie\n");
  gfx_print("wck\n");
  gfx_print("We've known each other for so long...\n");
  gfx_print("Why733\n");
  gfx_print("WiFiPunk\n");
  gfx_print("WillC\n");
  gfx_print("WillyCrash\n");
  gfx_print("Wiqwaq\n");
  gfx_print("Wishbone\n");
  gfx_print("XCProof\n");
  gfx_print("XORRO\n");
  gfx_print("XThree13\n");
  gfx_print("Yak\n");
  gfx_print("yes\n");
  gfx_print("Yoskay\n");
  gfx_print("Youbetcha77\n");
  gfx_print("Your heart's been aching but you're too shy to say it\n");
  gfx_print("Zanzibar\n");
  gfx_print("Zaphod Beeblebrox\n");
  gfx_print("ZFASEL\n");
  gfx_print("zoomequipd\n");
}

/**
 * @brief Display shouts screen for the badge
 */
void ui_shouts() {
  cursor_coord_t cursor = {0, 0};

  while (1) {
    gfx_fill_screen(COLOR_BLACK);
    gfx_color_set(COLOR_GREEN);
    gfx_font_set(font_large);
    gfx_cursor_set(cursor);
    gfx_print("Shouts\n");
    gfx_font_set(font_small);
    gfx_print(
        "Wives, Girlfriends, Kids,\nDogs, Cats, & Birds for putting up with "
        "this year-\nround\n\n");
    gfx_print("#badgelife for emotional \nsupport\n\n");
    gfx_print("Everyone from DC24 & DC25\n\n");
    gfx_print("Dialog Semiconductor: for\nGreenpaks\n\n");
    gfx_print("Macrofab: In N Out is\nsuperior to Whataburger\n\n");
    gfx_print("@justinmeza: for LOLCODE\n\n");
    gfx_print("g8, manchmod, b3gt: You\nguys rock!\n\n");
    gfx_print(
        "Kickstarter backers:\nWithout you this wouldn't\nbe possible!\n\n");
    ks_backer_credits();
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
    } else if (btn_left()) {
      btn_clear();
      break;
    }
  }
}

/**
 * @brief Draw a text box supporting scrolling and word wrap
 * @return if truncated
 */
bool ui_textbox(int16_t x,
                int16_t y,
                uint16_t w,
                uint16_t h,
                int16_t scroll_y,
                char* text,
                color_565_t fg,
                color_565_t bg) {
  uint8_t padding = 4;
  cursor_coord_t cursor;

  // Setup the area
  area_t area = {x + padding, y + padding, x + w - padding, y + h - padding};

  gfx_fill_rect(x, y, w, h, bg);
  gfx_draw_rect(x, y, w, h, fg);
  gfx_font_set(font_medium);

  // Setup the cursor
  gfx_cursor_area_set(area);
  gfx_color_set(fg);
  cursor.x = x + padding;
  cursor.y = y + padding + scroll_y + 3;
  gfx_cursor_set(cursor);
  uint8_t font_width = gfx_font_width();

  // Wrap the text assuming fixed width
  // Copy text onto heap i case it's a const
  char* text_heap = (char*)util_heap_alloc_ext(strlen(text) + 1);
  memcpy(text_heap, text, strlen(text) + 1);
  __wrap_text(text_heap, (area.xe - area.xs) / font_width);
  gfx_print(text_heap);
  free(text_heap);

  // f any rows truncated, indicate to the user
  if (gfx_cursor_get().y > area.ye) {
    gfx_draw_triangle(x + w - 8, y + h - 8, x + w - 4, y + h - 8, x + w - 6,
                      y + h - 4, fg);
  }

  // If we've scrolled down at all, indicate to user
  if (scroll_y < 0) {
    gfx_draw_triangle(x + w - 8, y + 8, x + w - 4, y + 8, x + w - 6, y + 4, fg);
  }

  area.xs = x;
  area.xe = x + w;
  area.ys = y;
  area.ye = y + h;
  gfx_cursor_area_set(area);
  cursor_coord_t c = gfx_cursor_get();
  uint16_t font_h = gfx_font_height();

  gfx_cursor_area_reset();
  return c.y > (area.ye - font_h);
}

/**
 * @brief Show a clock full screen
 */
void ui_time() {
  char buffer[80];
  time_t raw_time;
  struct tm* timeinfo;
  int16_t x, y;
  uint16_t w, h;
  cursor_coord_t cursor;
  char warning[] = "**Inaccurate time source";

  while (1) {
    gfx_fill_screen(COLOR_BLACK);

    time(&raw_time);
    timeinfo = localtime(&raw_time);

    gfx_font_set(font_large);
    gfx_color_set(COLOR_GREEN);

    // Format, center, and print current time UTC
    strftime(buffer, sizeof(buffer), "%H:%M:%S UTC", timeinfo);
    gfx_text_bounds(buffer, 0, 0, &x, &y, &w, &h);
    cursor.x = (LCD_WIDTH - w + x) / 2;
    cursor.y = (LCD_HEIGHT + y) / 2;
    gfx_cursor_set(cursor);
    gfx_print(buffer);

    // Format, center, and print current day UTC
    strftime(buffer, sizeof(buffer), "%F", timeinfo);
    gfx_text_bounds(buffer, 0, 0, &x, &y, &w, &h);
    cursor.x = (LCD_WIDTH - w + x) / 2;
    cursor.y = (LCD_HEIGHT - h - h + y - 5) / 2;
    gfx_cursor_set(cursor);
    gfx_print(buffer);

    gfx_font_set(font_small);
    gfx_color_set(COLOR_LIGHTGREY);
    cursor.x = 0;
    cursor.y = 0;
    gfx_cursor_set(cursor);
    sprintf(buffer, "Stratum: %d", time_manager_stratum_get());
    gfx_print(buffer);

    // Print a warning if stratum is high
    if (time_manager_stratum_get() > 8) {
      cursor.x = 0;
      cursor.y = LCD_HEIGHT - gfx_font_height();
      gfx_cursor_set(cursor);
      gfx_print(warning);
    }

    gfx_push_screen_buffer();

    if (btn_left()) {
      DELAY(100);
      btn_clear();
      return;
    }

    DELAY(200);
  }
}