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
const static char* TAG = "MRMEESEEKS::HAL_I2C";
static SemaphoreHandle_t m_mutex;

bool hal_i2c_init() {
  ESP_LOGD(TAG, "Initializing I2C #1");

  m_mutex = xSemaphoreCreateMutex();

  i2c_config_t conf1;
  conf1.mode = I2C_MODE_MASTER;
  conf1.sda_io_num = I2C_1_SDA;
  conf1.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf1.scl_io_num = I2C_1_SCL;
  conf1.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf1.master.clk_speed = I2C_1_FREQ_HZ;
  i2c_param_config(I2C_NUM_0, &conf1);
  esp_err_t result =
      i2c_driver_install(I2C_NUM_0, conf1.mode, I2C_MASTER_RX_BUF_DISABLE,
                         I2C_MASTER_TX_BUF_DISABLE, 0);

  if (result != ESP_OK) {
    ESP_LOGD(TAG, "Unable to initialize I2C #1, ret = %d", result);
    return false;
  }

  ESP_LOGD(TAG, "Initializing I2C #2");
  i2c_config_t conf2;
  conf2.mode = I2C_MODE_MASTER;
  conf2.sda_io_num = I2C_2_SDA;
  conf2.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf2.scl_io_num = I2C_2_SCL;
  conf2.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf2.master.clk_speed = I2C_2_FREQ_HZ;
  i2c_param_config(I2C_NUM_1, &conf2);
  result = i2c_driver_install(I2C_NUM_1, conf2.mode, I2C_MASTER_RX_BUF_DISABLE,
                              I2C_MASTER_TX_BUF_DISABLE, 0);

  if (result != ESP_OK) {
    ESP_LOGD(TAG, "Unable to initialize I2C #2, ret = %d", result);
    return false;
  }

  return true;
}

bool hal_i2c_write_byte(uint8_t master, uint8_t address, uint8_t data) {
  esp_err_t ret = 0;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1);
  i2c_master_write_byte(cmd, data, 1);
  i2c_master_stop(cmd);

  if (!xSemaphoreTake(m_mutex, I2C_MAX_DELAY)) {
    i2c_cmd_link_delete(cmd);
    return false;
  }

  ret = i2c_master_cmd_begin(master, cmd, 500 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  xSemaphoreGive(m_mutex);

  switch (ret) {
    case ESP_ERR_TIMEOUT:
      ESP_LOGD(TAG, "Error writing byte %01x to address %#01x, reason: TIMEOUT",
               data, address);
      break;
    case ESP_FAIL:
      ESP_LOGD(TAG,
               "Error writing byte %01x to address %#01x, reason: FAIL, NO ACK",
               data, address);
      break;
    case ESP_OK:
      break;
    default:
      ESP_LOGD(
          TAG,
          "Error writing byte %01x to address %#01x, reason: Unknown (%#01x)",
          data, address, ret);
      break;
  }

  return (ret == ESP_OK);
}

bool hal_i2c_write_reg_byte(uint8_t master,
                            uint8_t address,
                            uint8_t reg,
                            uint8_t data) {
  esp_err_t ret = 0;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1);
  i2c_master_write_byte(cmd, reg, 1);
  i2c_master_write_byte(cmd, data, 1);
  i2c_master_stop(cmd);

  if (!xSemaphoreTake(m_mutex, I2C_MAX_DELAY)) {
    i2c_cmd_link_delete(cmd);
    return false;
  }

  ret = i2c_master_cmd_begin(master, cmd, 500 / portTICK_RATE_MS);

  i2c_cmd_link_delete(cmd);
  xSemaphoreGive(m_mutex);

  switch (ret) {
    case ESP_ERR_TIMEOUT:
      ESP_LOGD(TAG,
               "Error writing byte 0x%02x to address 0x%02x register 0x%02x, "
               "reason: TIMEOUT",
               data, address, reg);
      break;
    case ESP_FAIL:
      // ESP_LOGD(TAG, "Error writing byte %01x to %01x, reason: FAIL, NO ACK",
      // data, reg)
      // ;
      break;
    case ESP_OK:
      break;
    default:
      ESP_LOGD(TAG, "Error writing byte %01x to %01x, reason: Unknown (%#1x)",
               data, reg, ret);
      break;
  }

  return (ret == ESP_OK);
}

bool hal_i2c_write_bytes(uint8_t master,
                         uint8_t address,
                         uint8_t reg,
                         uint8_t* data,
                         size_t length) {
  esp_err_t ret = 0;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, 1);
  i2c_master_write_byte(cmd, reg, 1);
  i2c_master_write(cmd, data, length, true);
  i2c_master_stop(cmd);

  if (!xSemaphoreTake(m_mutex, I2C_MAX_DELAY)) {
    i2c_cmd_link_delete(cmd);
    return false;
  }
  ret = i2c_master_cmd_begin(master, cmd, 500 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  xSemaphoreGive(m_mutex);

  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Error writing I2C: %#1x", ret);
    return false;
  }

  return (ret == ESP_OK);
}

void hal_i2c_read_bytes(uint8_t master,
                        uint8_t address,
                        uint8_t* data,
                        uint16_t len) {
  esp_err_t ret = 0;

  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
  i2c_master_start(cmd2);
  i2c_master_write_byte(cmd2, (address << 1) | I2C_MASTER_READ, 1);
  if (len > 1) {
    i2c_master_read(cmd2, data, 1, 1);
  }
  i2c_master_read(cmd2, data, 1, 0);
  i2c_master_stop(cmd2);

  if (!xSemaphoreTake(m_mutex, I2C_MAX_DELAY)) {
    i2c_cmd_link_delete(cmd2);
    return false;
  }

  ret = i2c_master_cmd_begin(master, cmd2, 500 / portTICK_RATE_MS);
  switch (ret) {
    case ESP_ERR_TIMEOUT:
      ESP_LOGD(TAG, "Error reading response, reason: TIMEOUT");
      break;
    case ESP_FAIL:
      ESP_LOGD(TAG, "Error reading response, reason: FAIL, NO ACK");
      break;
    case ESP_OK:
      break;
    default:
      ESP_LOGD(TAG, "Error reading response, reason: Unknown (%#1x)", ret);
      break;
  }
  i2c_cmd_link_delete(cmd2);
  xSemaphoreGive(m_mutex);
}

uint8_t hal_i2c_read_byte(uint8_t master, uint8_t address) {
  esp_err_t ret = 0;
  uint8_t data = 0x0;

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, true);
  i2c_master_read_byte(cmd, &data, 1);  // 1 == no ack
  i2c_master_stop(cmd);

  if (!xSemaphoreTake(m_mutex, I2C_MAX_DELAY)) {
    i2c_cmd_link_delete(cmd);
    return false;
  }

  ret = i2c_master_cmd_begin(master, cmd, 500 / portTICK_RATE_MS);
  switch (ret) {
    case ESP_ERR_TIMEOUT:
      ESP_LOGD(TAG, "Error reading byte from %#01x, reason: TIMEOUT", address);
      break;
    case ESP_FAIL:
      ESP_LOGD(TAG,
               "Error reading byte from address %#01x, reason: FAIL, NO ACK",
               address);
      break;
    case ESP_OK:
      break;
    default:
      ESP_LOGD(TAG,
               "Error reading byte from address %#01x, reason: Unknown (%#01x)",
               address, ret);
      break;
  }
  i2c_cmd_link_delete(cmd);
  xSemaphoreGive(m_mutex);

  return data;
}

void hal_i2c_read_reg_bytes(uint8_t master,
                            uint8_t address,
                            uint8_t reg,
                            uint8_t* data,
                            uint16_t len) {
  esp_err_t ret = 0;
  if (len == 0) {
    return;
  }

  i2c_cmd_handle_t cmd = i2c_cmd_link_create();
  i2c_master_start(cmd);
  i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, true);
  i2c_master_write_byte(cmd, reg, true);
  i2c_master_stop(cmd);

  if (!xSemaphoreTake(m_mutex, I2C_MAX_DELAY)) {
    i2c_cmd_link_delete(cmd);
    return false;
  }

  ret = i2c_master_cmd_begin(master, cmd, 500 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);

  switch (ret) {
    case ESP_ERR_TIMEOUT:
      ESP_LOGD(TAG,
               "Error writing to register address 0x%02x register 0x%02x for "
               "read, reason: TIMEOUT",
               address, reg);
      break;
    case ESP_FAIL:
      ESP_LOGD(TAG,
               "Error writing to register address 0x%02x register 0x%02x for "
               "read, reason: FAIL, NO ACK",
               address, reg);
      break;
    case ESP_OK:
      break;
    default:
      ESP_LOGD(TAG,
               "Error writing to register address 0x%02x register 0x%02x for "
               "read, reason: Unknown (%#1x)",
               address, reg, ret);
      break;
  }

  cmd = i2c_cmd_link_create();
  if (data) {
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, 1);
    if (len > 1) {
      i2c_master_read(cmd, data, len - 1, 0);
    }
    i2c_master_read(cmd, data + len - 1, 1, 1);
    i2c_master_stop(cmd);
  }

  ret = i2c_master_cmd_begin(master, cmd, 1000 / portTICK_RATE_MS);
  i2c_cmd_link_delete(cmd);
  xSemaphoreGive(m_mutex);

  switch (ret) {
    case ESP_ERR_TIMEOUT:
      ESP_LOGD(TAG,
               "Error reading register response address 0x%02x register 0x%02x "
               "for read, reason: TIMEOUT",
               address, reg);
      break;
    case ESP_FAIL:
      ESP_LOGD(TAG,
               "Error reading register response address 0x%02x register 0x%02x "
               "for read, reason: FAIL, NO ACK",
               address, reg);
      break;
    case ESP_OK:
      break;
    default:
      ESP_LOGD(TAG,
               "Error reading register response address 0x%02x register 0x%02x "
               "for read, reason: Unknown (%#1x)",
               address, reg, ret);
      break;
  }
}

/**
 * @brief Simple hardware abstraction for I2C on ESP32. This function reads a
 * single byte from a specific register of an I2C device.
 * @param master : ESP32 master number for the I2C bus to use
 * @param address : The 7-bit address of the I2C device to read
 * @param reg : The register in the device to read
 * @return The byte read from the register. Careful, default return value is 0
 * even if there is an error
 */
uint8_t hal_i2c_read_reg_byte(uint8_t master, uint8_t address, uint8_t reg) {
  esp_err_t ret = 0;
  uint8_t data = 0x0;

  i2c_cmd_handle_t cmd1 = i2c_cmd_link_create();
  i2c_master_start(cmd1);
  i2c_master_write_byte(cmd1, (address << 1) | I2C_MASTER_WRITE, 1);
  i2c_master_write_byte(cmd1, reg, 1);
  i2c_master_stop(cmd1);

  if (!xSemaphoreTake(m_mutex, I2C_MAX_DELAY)) {
    i2c_cmd_link_delete(cmd1);
    return false;
  }

  ret = i2c_master_cmd_begin(master, cmd1, 1000 / portTICK_RATE_MS);
  switch (ret) {
    case ESP_ERR_TIMEOUT:

      ESP_LOGD(TAG,
               "Error writing to register address 0x%02x reg 0x%02x for read, "
               "reason: TIMEOUT",
               address, reg);
      break;
    case ESP_FAIL:
      ESP_LOGD(TAG,
               "Error writing to register address 0x%02x reg 0x%02x for read, "
               "reason: NO ACK",
               address, reg);
      break;
    case ESP_OK:
      break;
    default:
      ESP_LOGD(TAG,
               "Error writing to register address 0x%02x reg 0x%02x for read, "
               "reason: Unknown (%#1x)",
               address, reg, ret);
      break;
  }
  i2c_cmd_link_delete(cmd1);

  DELAY(20);

  i2c_cmd_handle_t cmd2 = i2c_cmd_link_create();
  i2c_master_start(cmd2);
  i2c_master_write_byte(cmd2, (address << 1) | I2C_MASTER_READ, 1);
  i2c_master_read_byte(cmd2, &data, 1);  // 1 == no ack
  i2c_master_stop(cmd2);

  ret = i2c_master_cmd_begin(master, cmd2, 1000 / portTICK_RATE_MS);

  if (ret != ESP_OK) {
    ESP_LOGD(TAG, "Error reading address 0x%02x reg 0x%02x, ret=%#1x", address,
             reg, ret);
  }
  i2c_cmd_link_delete(cmd2);
  xSemaphoreGive(m_mutex);

  return data;
}
