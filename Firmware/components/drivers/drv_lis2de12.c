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

const static char* TAG = "MRMEESEEKS::LIS2DE12";
static uint8_t m_address = ACCEL_I2C_ADDR_1;
static volatile accel_axis_t m_accel_axis;
static bool m_accel_available = false;

/**
 * Regularly poll the acceleromter to avoid doing too much with the I2C bus
 */
static void IRAM_ATTR __accel_poll_task(void* parameters) {
  ESP_LOGD(TAG, "Starting accelerometer task");
  uint8_t* data = (uint8_t*)util_heap_alloc_ext(6);

  // 10 hz timer
  TickType_t ticks = (1000 / 10) / portTICK_PERIOD_MS;

  while (1) {
    if (!m_accel_available) {
      break;
    }

    TickType_t start = xTaskGetTickCount();

    // Read all 6 bytes (2 per axis) starting at X_LOW. Accelerometer should
    // auto increment
    hal_i2c_read_reg_bytes(
        ACCEL_I2C_MASTER_NUM, m_address,
        LIS2DE12_REG_X_OUT_L |
            0x80,  // 0x80 modifies the register address to indicate multiple
                   // bytes to be read. See datasheet.
        data,      // Store in data
        6);        // Read 6 bytes

    // Combine the data into 16 bit values
    int16_t y = (int16_t)((((uint16_t)data[1]) << 8) | data[0]) >> 8;
    int16_t x = (int16_t)((((uint16_t)data[3]) << 8) | data[2]) >> 8;
    int16_t z = (int16_t)((((uint16_t)data[5]) << 8) | data[4]) >> 8;

    m_accel_axis.x = MAX(MIN(INT8_MAX, x), INT8_MIN);
    m_accel_axis.y = MAX(MIN(INT8_MAX, y), INT8_MIN);
    m_accel_axis.z = MAX(MIN(INT8_MAX, z), INT8_MIN);

    vTaskDelayUntil(&start, ticks);
  }

  // Just in case
  free(data);
  vTaskDelete(NULL);
}

bool drv_lis2de12_init() {
  uint8_t who_am_i = hal_i2c_read_reg_byte(ACCEL_I2C_MASTER_NUM, m_address,
                                           LIS2DE12_REG_WHO_AM_I);
  if (who_am_i == 0) {
    m_address = ACCEL_I2C_ADDR_2;
    ESP_LOGD(TAG, "Trying other address");
    who_am_i = hal_i2c_read_reg_byte(ACCEL_I2C_MASTER_NUM, m_address,
                                     LIS2DE12_REG_WHO_AM_I);
  }
  ESP_LOGD(TAG, "Who am I response = %#1x", who_am_i);

  if (who_am_i != LIS2DE12_REG_WHO_AM_I_RESP) {
    post_state_get()->accelerometer_ack = false;
    ESP_LOGE(TAG, "Invalid whoami response from LIS2DE12 %#1x", who_am_i);
    return false;
  }

  m_accel_available = true;

  post_state_get()->accelerometer_ack = true;

  // 10hz ODR and XYZ enable
  hal_i2c_write_reg_byte(
      ACCEL_I2C_MASTER_NUM, m_address, LIS2DE12_CTRL_REG1,
      LIS2DE12_CTRL_REG1_ODR_10HZ
          //			| LIS2DE12_CTRL_REG1_LPEN	//Must be set
          // for "correct operation" according to datasheet
          | LIS2DE12_CTRL_REG1_X_EN | LIS2DE12_CTRL_REG1_Y_EN |
          LIS2DE12_CTRL_REG1_Z_EN);  // Enable 3 axis

  // Enable HPF for interrupt purposes (0.2hz)
  hal_i2c_write_reg_byte(
      ACCEL_I2C_MASTER_NUM, m_address, LIS2DE12_CTRL_REG2,
      LIS2DE12_CTRL_REG2_HPM_AUTORESET | LIS2DE12_CTRL_REG2_HP_IA1);

  // Enable Block data update mode
  hal_i2c_write_reg_byte(ACCEL_I2C_MASTER_NUM, m_address, LIS2DE12_REG_CTRL4,
                         LIS2DE12_REG_CTRL4_BDU | LIS2DE12_REG_CTRL4_FS_2G);

  // Select FIFO mode
  hal_i2c_write_reg_byte(
      ACCEL_I2C_MASTER_NUM, m_address, LIS2DE12_FIFO_CTRL_REG,
      LIS2DE12_FIFO_CTRL_REG_BYPASS | LIS2DE12_FIFO_CTRL_REG_TR_INT2);

  ESP_LOGD(TAG, "LIS2DE12 Initialized");

  return true;
}

/**
 * Get the latest accelerometer data
 *
 * @return accel_axis_t Struct storing x, y, and z data
 */
accel_axis_t drv_lis2de12_get() {
  return m_accel_axis;
}

uint16_t drv_lis2de12_tilt_get() {
  if (!m_accel_available) {
    return 0;
  }

  accel_axis_t axis = drv_lis2de12_get();
  double angle = atan2((double)axis.y, (double)axis.x);
  if (angle < 0) {
    angle += 2 * M_PI;
  }
  return (uint16_t)round(angle * 180 / M_PI);
}

void drv_lis2de12_poll_task_start() {
  if (m_accel_available) {
    xTaskCreate(__accel_poll_task, "Accelerometer", 4096, NULL,
                TASK_PRIORITY_LOW, NULL);
  }
}
