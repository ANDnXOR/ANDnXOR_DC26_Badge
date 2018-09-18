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

#define BTN_ISR_FLAG_DEFAULT 0

static const char* TAG = "BTN";
static bool m_allow_user_input = true;
static volatile uint8_t m_button_state = 0;
static bool m_randomize_buttons = false;
static xQueueHandle m_evt_queue = NULL;

static void __greenpak_unlock_popup_task(void* parameters) {
  DELAY(200);
  btn_clear();
  ui_popup_info("POWER UP! 500 botnet points!");
  btn_clear();
  vTaskDelete(NULL);
}

static IRAM_ATTR void __button_handler(void* arg) {
  uint32_t mask = (uint32_t)arg;
  xQueueSendFromISR(m_evt_queue, &mask, NULL);
}

static void __btn_interrupt(void* arg) {
  uint32_t mask;
  while (1) {
    // Block until an interrupt is received from greenpak or 100ms
    xQueueReceive(m_evt_queue, &mask, 200 / portTICK_PERIOD_MS);

    if (m_allow_user_input) {
      // Check for seekrit combo
      if (drv_greenpak_combo_read() > 0) {
        uint8_t unlock = state_unlock_get();
        //Only do greenpak unlock if they haven't done it
        if ((unlock & UNLOCK_GREENPAK_COMBO) == 0) {
          unlock |= UNLOCK_GREENPAK_COMBO;
          state_unlock_set(unlock);
          state_save_indicate();
          xTaskCreate(__greenpak_unlock_popup_task, "GP Unlock", 4000, NULL,
                      TASK_PRIORITY_MEDIUM, NULL);
        }
      } else {
        // Read button state
        if (m_randomize_buttons) {
          m_button_state = 1 << util_random(0, 8);
        } else {
          m_button_state = drv_greenpak_button_state_read();
        }

        // Software check for screenshot key combo
        if (btn_a() && btn_b() && drv_sd_mounted()) {
          util_screenshot();
          btn_clear();
        }
      }
    }

    // Clear out the greenpak ISR queue so we don't hammer I2C
    xQueueReset(m_evt_queue);
  }

  // We should never get here but just in case
  vTaskDelete(NULL);
}

/**
 * @brief Enable or disable user input
 * @param user_input : set to true to allow user input
 */
void btn_allow_user_input(bool user_input) {
  m_allow_user_input = user_input;
}

/**
 * @brief Clear the current button state
 */
void btn_clear() {
  m_button_state = 0;
}

/**
 * Initialize the button high level API
 */
void btn_init() {
  gpio_config_t io_conf;

  // Interrupt on falling edge
  io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
  // bit mask of the pins
  io_conf.pin_bit_mask = GPIO_BTN_MASK;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 0;
  io_conf.pull_down_en = 1;
  gpio_config(&io_conf);

  // Create the queue to send data out of the ISR
  m_evt_queue = xQueueCreate(1, sizeof(uint32_t));
  // start gpio task
  xTaskCreatePinnedToCore(__btn_interrupt, "Button task", 3000, NULL,
                          TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);

  ESP_LOGD(TAG, "Adding button ISRs");

  // Hook ISRs for buttons
  gpio_isr_handler_add((gpio_num_t)GPIO_GREENPAK_ISR, __button_handler,
                       (void*)BUTTON_GREENPAK_ISR_MASK);

  ESP_LOGD(TAG, "Buttons Initialized");
}

inline uint8_t btn_state() {
  return m_button_state;
}

inline bool btn_a() {
  if (drv_ili9225_is_inverted()) {
    return (m_button_state & BUTTON_MASK_B) > 0;
  } else {
    return (m_button_state & BUTTON_MASK_A) > 0;
  }
}

inline bool btn_b() {
  if (drv_ili9225_is_inverted()) {
    return (m_button_state & BUTTON_MASK_A) > 0;
  } else {
    return (m_button_state & BUTTON_MASK_B) > 0;
  }
}

inline bool btn_c() {
  return (m_button_state & BUTTON_MASK_C) > 0;
}

inline bool btn_up() {
  if (drv_ili9225_is_inverted()) {
    return (m_button_state & BUTTON_MASK_DOWN) > 0;
  } else {
    return (m_button_state & BUTTON_MASK_UP) > 0;
  }
}

inline bool btn_down() {
  if (drv_ili9225_is_inverted()) {
    return (m_button_state & BUTTON_MASK_UP) > 0;
  } else {
    return (m_button_state & BUTTON_MASK_DOWN) > 0;
  }
}

inline bool btn_left() {
  if (drv_ili9225_is_inverted()) {
    return (m_button_state & BUTTON_MASK_RIGHT) > 0;
  } else {
    return (m_button_state & BUTTON_MASK_LEFT) > 0;
  }
}

inline bool btn_right() {
  if (drv_ili9225_is_inverted()) {
    return (m_button_state & BUTTON_MASK_LEFT) > 0;
  } else {
    return (m_button_state & BUTTON_MASK_RIGHT) > 0;
  }
}

/**
 * @brief Set whether or not to randomize buttons
 */
void btn_randomize(bool randomize) {
  m_randomize_buttons = randomize;
}

/**
 * @brief Block until button press
 */
uint8_t btn_wait() {
  uint8_t button = 0;

  while (button == 0) {
    DELAY(10);
    button = btn_state();
    YIELD();
  }
  return button;
}

/**
 * @brief Block until button press with a max wait time
 */
uint8_t btn_wait_max(uint32_t max_wait_ms) {
  uint8_t button = 0;
  uint32_t end_time = MILLIS() + max_wait_ms;

  while (button == 0 && MILLIS() < end_time) {
    DELAY(10);
    button = btn_state();
    YIELD();
  }
  return button;
}
