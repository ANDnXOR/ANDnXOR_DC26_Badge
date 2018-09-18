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
#include "drv_config.h"

#define GREENPAK_BUFFER_DISABLE 0
#define GREENPAK_DLY_ADDR 0xF2
#define GREENPAK_BAGEL_BIT_ADDR 1975
#define GREENPAK_TOAST_BIT_ADDR 1922
#define GREENPAK_COMBO_LUT_ADDR 1932

const static char* TAG = "MRMEESEEKS::Greenpak";

void drv_greenpak_flash(char* path) {
  uint32_t start = MILLIS();

  uint32_t fsize = util_file_size(path);
  ESP_LOGD(TAG, "Opening '%s', filesize=%d bytes.", path, fsize);
  FILE* file = fopen(path, "r");
  if (file == NULL) {
    return;
  }

  uint8_t data[4096];
  fread(data, 1, 4096, file);
  fclose(file);

  data[4095] = 0;

  const static char* tag = "<nvmData registerLenght=\"2048\">";
  const static char* close = "</nvmData>";
  char* nvm_data = strstr((char*)data, tag) + strlen(tag);
  char* end = strstr(nvm_data, close);
  end[0] = '\0';

  uint8_t nvm[256];
  memset(nvm, 0, 256);
  for (uint16_t i = 0; i < 256; i++) {
    char* space = strstr(nvm_data, " ");
    nvm[i] = strtol(nvm_data, NULL, 16);
    ESP_LOGD(TAG, "byte[%d] = 0x%02X", i, nvm[i]);
    nvm_data = space + 1;
    hal_i2c_write_reg_byte(GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, i,
                           nvm[i]);
  }

  ESP_LOGD(TAG, "Flashing Greenpak");
  //	hal_i2c_write_bytes(GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, 0x00,
  // nvm, 256);
  uint32_t delta = MILLIS() - start;
  ESP_LOGD(TAG, "Done. Time = %d ms", delta);

  return;
}

void drv_greenpak_init() {
  post_state_t* post = post_state_get();
  // BUG FIX! Pickle Rick Greenpaks have misconfigured Pin 10 (button ISR) This
  // fixes that.
  post->greenpak_ack = hal_i2c_write_reg_byte(GREENPAK_I2C_MASTER_NUM,
                                              GREENPAK_I2C_ADDR, 0x88, 0x00);

  // Ensure greenpak has certain bytes
  uint8_t testa = hal_i2c_read_reg_byte(
      GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, 0x23);  // Should be 0x10
  uint8_t testb = hal_i2c_read_reg_byte(
      GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, 0x40);  // Should be 0x17
  uint8_t testc = hal_i2c_read_reg_byte(
      GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, 0xFF);  // Should be 0xA5

  ESP_LOGD(TAG, "Greenpak test results 0x23=0x%02x 0x40=0x%02x 0xFF=0x%02x",
           testa, testb, testc);

  post->greenpak_read_memory =
      (testa == 0x10) && (testb == 0x17) && (testc == 0xA5);
}

void drv_greenpak_button_poll() {
  while (1) {
    uint8_t state = drv_greenpak_button_state_read();
    ESP_LOGD(TAG, "Current greenpak state = %#01x", state);
    DELAY(2000);
  }
}

uint8_t drv_greenpak_button_state_read() {
  uint8_t state = hal_i2c_read_reg_byte(GREENPAK_I2C_MASTER_NUM,
                                        GREENPAK_I2C_ADDR, GREENPAK_DLY_ADDR);
  if (state == 0xFF || state == 0x00) {
    return 0;
  }

  // This will read the state of all 7 CNT blocks, however, lowest bit also
  // happens to be the
  // output value of the overall button state (an OR of all buttons, 1 =
  // pressed)
  // Since the CNT blocks are low when button is pressed, need to invert to 1s
  // and mask out the
  // overall button state (which we should be reading over GPIO ISR elsewhere)
  return (~state) & 0xFE;
}

/**
 * @brief Read state of the combo LUT output
 */
uint8_t drv_greenpak_combo_read() {
  uint8_t addr = GREENPAK_COMBO_LUT_ADDR / 8;
  uint8_t shift = GREENPAK_COMBO_LUT_ADDR - (addr * 8);
  uint8_t byte =
      hal_i2c_read_reg_byte(GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, addr);
  return (byte >> shift) & 1;
}

/**
 * @brief Read the bagel pin.
 */
uint8_t drv_greenpak_read_bagel_pin() {
  uint8_t addr = GREENPAK_BAGEL_BIT_ADDR / 8;
  uint8_t shift = GREENPAK_BAGEL_BIT_ADDR - (addr * 8);
  uint8_t byte =
      hal_i2c_read_reg_byte(GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, addr);
  ESP_LOGD(TAG, "%s byte value = 0x%02x after shift 0x%02x", __func__, byte,
           (byte >> shift) & 1);
  return (byte >> shift) & 1;
}

/**
 * @brief Read the toast pin.
 */
uint8_t drv_greenpak_read_toast_pin() {
  uint8_t addr = GREENPAK_TOAST_BIT_ADDR / 8;
  uint8_t shift = GREENPAK_TOAST_BIT_ADDR - (addr * 8);
  uint8_t byte =
      hal_i2c_read_reg_byte(GREENPAK_I2C_MASTER_NUM, GREENPAK_I2C_ADDR, addr);
  ESP_LOGD(TAG, "%s byte value = 0x%02x after shift 0x%02x", __func__, byte,
           (byte >> shift) & 1);
  return (byte >> shift) & 1;
}