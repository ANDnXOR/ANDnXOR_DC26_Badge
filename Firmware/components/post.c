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

const static char* TAG = "MRMEESEEKS::POST";
post_state_t m_state;

inline post_state_t* post_state_get() {
  return &m_state;
}

void post_dump() {
  ESP_LOGI(TAG, "========================= POST ===========================");
  ESP_LOGI(TAG, "Hey marajade, hope this helps!");
  ESP_LOGI(TAG, "\tSerial: 0x%04x", util_serial_get());
  ESP_LOGI(TAG, "\tAccelerometer ACK\t%d", m_state.accelerometer_ack);
  ESP_LOGI(TAG, "\tGreenpak ACK\t\t%d", m_state.greenpak_ack);
  ESP_LOGI(TAG, "\tGreenpak Flash\t\t%d", m_state.greenpak_read_memory);
  ESP_LOGI(TAG, "\tLED Driver ACK\t\t%d", m_state.led_driver_ack);
  ESP_LOGI(TAG, "==========================================================");
}


/**
 * @brief Display POST status on screen
 */
void post_screen() {
  post_state_t* post = post_state_get();
  cursor_coord_t cursor = {0, 0};
  gfx_fill_screen(COLOR_BLACK);
  gfx_font_set(font_large);
  gfx_color_set(COLOR_GREEN);
  gfx_cursor_set(cursor);
  gfx_print("POST");

  // Accelermeter
  cursor.x = 0;
  cursor.y = 40;
  gfx_cursor_set(cursor);
  gfx_color_set(COLOR_GREEN);
  gfx_font_set(font_medium);
  gfx_print("Accelerometer..........");
  if (post->accelerometer_ack == 1) {
    gfx_print("Y\n");
  } else {
    gfx_color_set(COLOR_RED);
    gfx_print("N\n");
  }

  // Greenpak
  gfx_color_set(COLOR_GREEN);
  gfx_font_set(font_medium);
  gfx_print("Greenpak...............");
  if (post->greenpak_ack == 1) {
    gfx_print("Y\n");
  } else {
    gfx_color_set(COLOR_RED);
    gfx_print("N\n");
  }

  // LED Driver
  gfx_color_set(COLOR_GREEN);
  gfx_font_set(font_medium);
  gfx_print("LED Driver.............");
  if (post->led_driver_ack == 1) {
    gfx_print("Y\n");
  } else {
    gfx_color_set(COLOR_RED);
    gfx_print("N\n");
  }

  gfx_push_screen_buffer();
  btn_clear();
  btn_wait();
  DELAY(100);
  btn_clear();
}
