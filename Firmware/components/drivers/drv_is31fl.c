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

#define INIT_ATTEMPT_PERIOD_MS (10 * 1000) /* 10 second attempt period */

const static char* TAG = "MRMEESEEKS:IS31FL3736";
static bool m_initialized = false;
static uint32_t m_last_init_attempt = 0;

void drv_is31fl_gcc_set(uint8_t gcc) {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  // Lazy init
  drv_is31fl_init();

  drv_is31fl_set_page(ISSI_PAGE_FUNCTION);
  hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, ISSI_REG_GCC, gcc);
  drv_is31fl_set_page(ISSI_PAGE_PWM);
#endif
}

bool drv_is31fl_init() {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  // Avoid initializing too often
  if (m_last_init_attempt > 0) {
    if (m_initialized ||
        (MILLIS() - m_last_init_attempt) < INIT_ATTEMPT_PERIOD_MS) {
      return m_initialized;
    }
  }

  m_last_init_attempt = MILLIS();

  drv_is31fl_set_page(ISSI_PAGE_FUNCTION);
  if (!hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, ISSI_REG_CONFIG,
                              ISSI_REG_CONFIG_SSD_OFF)) {
    post_state_get()->led_driver_ack = false;
    return false;
  }
  DELAY(100);

  // Set global current control
  drv_is31fl_set_page(ISSI_PAGE_FUNCTION);
  hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, ISSI_REG_GCC, 0xFF);
  DELAY(10);

  // Turn off all LEDs
  drv_is31fl_set_page(ISSI_PAGE_LED);
  for (uint8_t i = 0; i <= 0x17; i++) {
    hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, i, 0x00);
  }
  DELAY(100);

  drv_is31fl_set_page(ISSI_PAGE_FUNCTION);
  if (!hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, ISSI_REG_CONFIG,
                              ISSI_REG_CONFIG_OSD | ISSI_REG_CONFIG_SSD_OFF)) {
    post_state_get()->led_driver_ack = false;
    return false;
  }

  DELAY(100);

  // PWM the LEDs back to 0
  drv_is31fl_set_page(ISSI_PAGE_PWM);
  for (uint8_t i = 0; i <= 0xBE; i += 1) {
    hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, i, 0x00);
  }

  DELAY(100);

  // Turn on all LEDs
  drv_is31fl_set_page(ISSI_PAGE_LED);
  for (uint8_t i = 0; i <= 0x17; i++) {
    hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, i, 0xFF);
  }

  DELAY(100);

  drv_is31fl_set_page(ISSI_PAGE_PWM);
  ESP_LOGD(TAG, "IS31FL3736 driver started.");

  m_initialized = true;
  post_state_get()->led_driver_ack = true;
  return true;
#endif

  return false;
}

void drv_is31fl_send_value(uint8_t address, uint8_t value) {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  // Lazy init
  drv_is31fl_init();
  if (m_initialized) {
    if (!hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, address,
                                value)) {
      m_initialized = false;
    }
  }
#endif
}

/**
 * Select page to write to
 */
void drv_is31fl_set_page(uint8_t page) {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, ISSI_REG_WRITE_LOCK,
                         ISSI_REG_WRITE_UNLOCK);
  hal_i2c_write_reg_byte(LED_I2C_MASTER_NUM, LED_I2C_ADDR, ISSI_REG_COMMAND,
                         page);
#endif
}
