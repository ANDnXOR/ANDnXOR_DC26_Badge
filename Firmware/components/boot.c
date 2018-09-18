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

const static char* TAG = "MRMEESEEKS::Boot";
uint16_t rainbow[13] = {COLOR_MAROON,    COLOR_RED,       COLOR_PINK,
                        COLOR_ORANGE,    COLOR_YELLOW,    COLOR_GREENYELLOW,
                        COLOR_GREEN,     COLOR_DARKGREEN, COLOR_BLUE,
                        COLOR_DARKBLUE,  COLOR_MAGENTA,   COLOR_PURPLE,
                        COLOR_NEONPURPLE};
static void __developer_mode();
static void __lol_loading();

/**
 * Put badge into developer mode which allows uploads over ymodem
 */
static void __developer_mode() {
  while (1) {
    led_eye_pulse_start();
    gfx_fill_screen(COLOR_RED);
    gfx_font_set(font_large);
    gfx_cursor_set((cursor_coord_t){0, 20});
    gfx_color_set(COLOR_BLACK);
    gfx_bg_transparent(true);
    gfx_print("Developer Mode\n");
    gfx_font_set(font_small);
    gfx_print(
        "Upload with Atom\n\nor\n\nstty -F /dev/ttyUSB0 115200\n\nsz --ymodem "
        "FILE > /dev/ttyUSB0 < /dev/ttyUSB0");
    gfx_push_screen_buffer();

    ymodem_listen();
    gfx_fill_screen(COLOR_BLACK);
    util_heap_stats_dump();
    led_eye_pulse_stop();
    DELAY(500);
  }
}

static void __lol_loading() {
  uint8_t i = 0;
  gfx_fill_screen(COLOR_BLACK);
  gfx_font_set(font_small);
  gfx_color_set(rainbow[i++]);
  gfx_cursor_set((cursor_coord_t){0, 34});
  gfx_print("LOLOLOLOLOLOLOLOLOLOLOLOLOL\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("O                         O\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("L                         L\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("O                         O\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("L                         L\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("O                         O\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("L       LOADING...        L\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("O                         O\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("L                         L\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("O                         O\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("L                         L\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("O                         O\n");
  gfx_color_set(rainbow[i++]);
  gfx_print("LOLOLOLOLOLOLOLOLOLOLOLOLOL\n");
  gfx_push_screen_buffer();
}

/**
 * @brief Block the user until SD card inserted
 */
static void __sd_check() {
  uint32_t end_time =
      MILLIS() + 5000;  // wait up to 5 seconds before blocking user
  while (!drv_sd_mounted()) {
    if (MILLIS() > end_time) {
      ui_popup_error(
          "No SD Card :(\n\nInsert now\n\nAny button or reset to continue.");
    }
    DELAY(500);
  }
}

/**
 * @brief Check if an update was recently completed
 */
static void __update_check() {
  if (util_file_exists("/sdcard/UPDATED")) {
    char buffer[64];
    sprintf(buffer, "Update complete! New version v%s.", VERSION);
    ui_popup_info(buffer);
    unlink("/sdcard/UPDATED");
  }
}

/**
 * @brief Do a normal boot
 */
__attribute__((__unused__)) static void __normal_boot() {
  led_eye_blink_delay(100);

  // Accelerometer polling task
  drv_lis2de12_poll_task_start();

  // Startup background task to monitor accelerometer
  gfx_invert_screen_task_start();

  // Startup background task to save data
  state_save_task_start();

  // Startup BLE GAP rotation task
  ble_rotate_task_start();

  // Startup BLE Nonce randomization task
  ble_nonce_task_start();

  // Quietly wait for BEER TIME!
  time_manager_beer_time_task_start();

  led_clear();

  led_eye_blink_stop();
  DELAY(100);
  led_eye_set(false);

#ifdef CONFIG_DEVELOPER_MODE
  // Run bootloader menu as a task so we keep main running in bg
  __developer_mode();
#else

  // Block until SD inserted
  __sd_check();

  // Check if recently updated
  __update_check();

  // Load badge state
  state_load();

  // Check bagel pin unlock
  util_bagel_pin_unlock_check();

  if (btn_down()) {
    gfx_fill_screen(COLOR_RED);
    gfx_push_screen_buffer();
    while (btn_down()) {
      // wait for button release
    }
    btn_clear();

    __developer_mode();
  } else {
    // Start the addons
    addons_task_start();

    // Intro video :)
    gfx_play_raw_file("/sdcard/bling/intro.raw", 0, 0, LCD_WIDTH, LCD_HEIGHT,
                      NULL, false, NULL);

    // Startup Console
    console_task_start();

    // Indicate in logs that things are running
    ESP_LOGI(TAG, "AND!XOR DC26 Badge Ready - %s (%s) - Now: %d", VERSION,
             IDF_VER, time_manager_now_sec());

    // Main menu time
    // static StaticTask_t task;
    // util_task_create(ui_menu_main, "MAIN", 8192, NULL, TASK_PRIORITY_MEDIUM,
    //                  &task);

    while (1) {
      ui_menu_main(NULL);
    }

    // Just in case
    esp_restart();

    // Should never happen
    while (1) {
      DELAY(50);
    }
  }
#endif
}

/**
 * @brief Test mode for Macrofab
 */
__attribute__((__unused__)) static void __test_boot() {
  ESP_LOGI(TAG, "Test Mode!");
  while (1) {
    led_set_all(255, 0, 0);
    led_show();
    DELAY(1000);
    if (btn_state() > 0)
      break;

    led_set_all(0, 255, 0);
    led_show();
    DELAY(1000);
    if (btn_state() > 0)
      break;

    led_set_all(0, 0, 255);
    led_show();
    DELAY(1000);
    if (btn_state() > 0)
      break;

    led_set_all(255, 255, 255);
    led_show();
    DELAY(1000);
    if (btn_state() > 0)
      break;
  }

  led_clear();

  // Test buttons
  while (1) {
    btn_wait();
    if (btn_left()) {
      for (uint8_t i = 0; i < 5; i++) {
        led_set(i, 255, 0, 0);
      }
    }
    if (btn_right()) {
      for (uint8_t i = 5; i < 10; i++) {
        led_set(i, 255, 0, 0);
      }
    }
    if (btn_up()) {
      for (uint8_t i = 10; i < 15; i++) {
        led_set(i, 255, 0, 0);
      }
    }
    if (btn_down()) {
      for (uint8_t i = 15; i < 19; i++) {
        led_set(i, 255, 0, 0);
      }
    }
    if (btn_a()) {
      for (uint8_t i = 19; i < 23; i++) {
        led_set(i, 255, 0, 0);
      }
    }
    if (btn_b()) {
      for (uint8_t i = 23; i < 27; i++) {
        led_set(i, 255, 0, 0);
      }
    }
    if (btn_c()) {
      for (uint8_t i = 27; i < 31; i++) {
        led_set(i, 255, 0, 0);
      }
    }

    led_show();
  }
}

/**
 * @brief Startup the badge
 */
void mrmeeseeks_bootloader() {
  led_eye_init();
  led_eye_blink_delay(400);
  led_eye_blink_start();

  util_nvs_init();

  // Enable ISRs
  gpio_install_isr_service(0);

  led_eye_blink_delay(300);

  drv_sd_init();
  DELAY(500);  // wait for SD to come up

  hal_i2c_init();
  drv_ili9225_backlight_set(0);
  btn_init();
  drv_lis2de12_init();
  led_eye_blink_delay(100);

  drv_greenpak_init();

  led_init();

  // Initialize the screen
  drv_ili9225_init();
  gfx_init();
  gfx_fill_screen(COLOR_BLACK);
  __lol_loading();

  util_heap_stats_dump();

  // Turn on the backlight now that we've filled the buffer
  drv_ili9225_backlight_set(200);

  // Load badge state
  state_init();

  ble_init();

  led_eye_blink_delay(200);

  // Necessary to install the UART driver
  uart_driver_install(UART_NUM_0, 1024 * 2, 1024 * 2, 0, NULL, 0);

  peers_init();

  botnet_init();

  util_update_global_brightness();
  led_clear();

  ESP_LOGD(TAG, "Dumping POST state");
  post_dump();

#ifndef CONFIG_BADGE_TYPE_TEST
  // Do normal startup
  __normal_boot();
#else
  __test_boot();
#endif

  // Everything should fall here on main task
  while (1) {
    DELAY(6000);
  }
}
