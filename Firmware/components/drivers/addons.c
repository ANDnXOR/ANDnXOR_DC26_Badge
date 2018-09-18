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

#define ADDONS_ANIMATION_TIME 60
#define ADDON_I2C_ADDRESS_7B 0x20

// MCP23017 registers assuming BANK=1
#define MCP23017_REG_IODIRA 0x00
#define MCP23017_REG_IODIRB 0x01
// Ports A and B GPIO latch state (aka output)
#define MCP23017_REG_OLATA 0x14
#define MCP23017_REG_OLATB 0x15

#define TRANSITION                                                    \
  if (time_manager_now_sec() >= transition_time) {                    \
    m_state = (m_state + 1) % addon_state_count;                      \
    transition_time = time_manager_now_sec() + ADDONS_ANIMATION_TIME; \
  }

#define WRITE_PORTA(value)                                                \
  if (!hal_i2c_write_reg_byte(ADDON_I2C_MASTER_NUM, ADDON_I2C_ADDRESS_7B, \
                              MCP23017_REG_OLATA, value)) {               \
    m_state = addon_state_searching;                                      \
  }
#define WRITE_PORTB(value)                                                \
  if (!hal_i2c_write_reg_byte(ADDON_I2C_MASTER_NUM, ADDON_I2C_ADDRESS_7B, \
                              MCP23017_REG_OLATB, value)) {               \
    m_state = addon_state_searching;                                      \
  }

// Define states for the addon FSM
typedef enum {
  addon_state_chase,
  addon_state_chase2,
  addon_state_kitt,
  addon_state_random,
  addon_state_count,
  addon_state_searching,
  addon_state_stopped
} addon_state_t;

__attribute__((__unused__)) const static char* TAG = "MRMEESEEKS::Addon";
static addon_state_t m_state = addon_state_searching;

void __addon_step(void* parameters) {
  int8_t direction = 1;
  uint8_t index = 0;
  uint32_t transition_time = 0;

  while (1) {
    switch (m_state) {
      // Searching for an addon
      case addon_state_searching:
        DELAY(1000);

        // This assumes POR which sets BANK to 0

        // Write to DIRA register
        bool result =
            hal_i2c_write_reg_byte(ADDON_I2C_MASTER_NUM, ADDON_I2C_ADDRESS_7B,
                                   MCP23017_REG_IODIRA, 0x00);

        // Found an addon, transition to chase animation
        if (result) {
          DELAY(50);
          // Port B DIRB write was a success, let's try Port B DIRB
          if (hal_i2c_write_reg_byte(ADDON_I2C_MASTER_NUM, ADDON_I2C_ADDRESS_7B,
                                     MCP23017_REG_IODIRB, 0x00)) {
            // That worked, start animating
            m_state = addon_state_chase2;
            transition_time = time_manager_now_sec() + ADDONS_ANIMATION_TIME;
          }
        }
        break;

      // Chase animation mode
      case addon_state_chase:
        DELAY(50);
        WRITE_PORTA((uint8_t)(1 << index));
        WRITE_PORTB((uint8_t)(1 << index));
        index = (index + 1) % 8;
        TRANSITION;
        break;

      // Chase animation mode, but more extreme
      case addon_state_chase2:
        DELAY(30);
        WRITE_PORTA((uint8_t)(1 << index));
        WRITE_PORTB((uint8_t)(1 << index));

        if (direction < 0) {
          if (index > 0) {
            index--;
          } else {
            index = 7;
          }
          if (util_random(0, 100) < 4) {
            direction = 1;
          }
        } else {
          index = (index + 1) % 8;
          if (util_random(0, 100) < 4) {
            direction = -1;
          }
        }

        TRANSITION;
        break;

        // Kit wiping back and forth
      case addon_state_kitt:
        DELAY(70);

        switch (index) {
          case 0:
            WRITE_PORTA(0x7E);
            WRITE_PORTB(0x00);
            break;
          case 1:
            WRITE_PORTA(0x81);
            WRITE_PORTB(0x00);
            break;
          case 2:
            WRITE_PORTA(0x00);
            WRITE_PORTB(0x81);
            break;
          case 3:
            WRITE_PORTA(0x00);
            WRITE_PORTB(0x42);
            break;
          case 4:
            WRITE_PORTA(0x00);
            WRITE_PORTB(0x24);
            break;
          case 5:
            WRITE_PORTA(0x00);
            WRITE_PORTB(0x08);
            break;
          case 6:
            WRITE_PORTA(0x00);
            WRITE_PORTB(0x10);
            break;
        }
        if (direction > 0) {
          index++;
          if (index >= 7) {
            direction = -1;
            index = 6;
          }
        } else {
          if (index > 0) {
            index--;
          } else {
            index = 0;
            direction = 1;
          }
        }
        TRANSITION;
        break;

      // Random LED mode
      case addon_state_random:
        DELAY(100);
        WRITE_PORTA(util_random(0, UINT8_MAX));
        WRITE_PORTB(util_random(0, UINT8_MAX));
        TRANSITION;
        break;

      // Do nothing
      case addon_state_stopped:
        DELAY(100);
        break;

      // Should never get here but if it does go back to searching mode
      case addon_state_count:
        m_state = addon_state_searching;
        break;
    }
  }
  vTaskDelete(NULL);
}

/**
 * @brief Start the addon
 */
void addons_start() {
  m_state = addon_state_searching;
}

/**
 * @brief Stop the addon
 */
void addons_stop() {
  m_state = addon_state_stopped;
  DELAY(100);
  WRITE_PORTA(0);
  WRITE_PORTB(0);
}

/**
 * @brief Start the addons task which handles plug and play and animation in the
 * background
 */
void addons_task_start() {
  xTaskCreatePinnedToCore(__addon_step, "Add-ons", 3000, NULL,
                          TASK_PRIORITY_ADDONS, NULL, APP_CPU_NUM);
}