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

const static char* TAG = "MRMEESEEKS::ILI9225";

static spi_device_handle_t m_spi;
static bool m_inverted = false;
static bool m_mirrored = false;
static SemaphoreHandle_t m_mutex;
static uint16_t m_rgb_order = ILI9225_ENTRY_MODE_BGR;

typedef struct {
  uint8_t cmd;
  uint8_t data[16];
  uint8_t databytes;  // No of data in data; bit 7 = delay after set; 0xFF = end
                      // of cmds.
} ili_init_cmd_t;

// Send a command to the ILI9341. Uses spi_device_transmit, which waits until
// the transfer is complete.
static inline void __cmd(uint8_t cmd) {
  esp_err_t ret;
  spi_transaction_t t;
  memset(&t, 0, sizeof(t));  // Zero out the transaction
  t.length = 8;
  t.tx_data[0] = cmd;
  t.flags = SPI_TRANS_USE_TXDATA;
  t.user = (void*)0;  // D/C needs to be set to 0

  ret = spi_device_transmit(m_spi, &t);  // Transmit!
  assert(ret == ESP_OK);                 // Should have had no issues.
}

/**
 * This function is called (in irq context!) just before a transmission starts.
 * It will
 * set the D/C line to the value indicated in the user field.
 */
static void IRAM_ATTR __spi_pre_callback(spi_transaction_t* t) {
  int dc = (int)t->user;
  gpio_set_level(LCD_PIN_DC, dc);
}

// Send data to the ILI9341. Uses spi_device_transmit, which waits until the
// transfer is complete.
static void __data(uint8_t* data, int len) {
  esp_err_t ret;
  spi_transaction_t t;
  if (len == 0)
    return;                  // no need to send anything
  memset(&t, 0, sizeof(t));  // Zero out the transaction
  t.length = len * 8;        // Len is in bytes, transaction length is in bits.
  t.tx_buffer = data;
  t.user = (void*)1;  // D/C needs to be set to 1

  ret = spi_device_transmit(m_spi, &t);  // Transmit!
  assert(ret == ESP_OK);                 // Should have had no issues.
}

// Send data to the ILI9341. Uses spi_device_transmit, which waits until the
// transfer is complete.
static void __data_16(uint16_t data) {
  uint16_t lsb = ((data) << 8) | ((data) >> 8);
  __data((uint8_t*)&lsb, 2);
}

static inline void __register(uint8_t cmd, uint16_t data) {
  __cmd(cmd);
  __data_16(data);
}

/**
 * @brief Update the registers in the screen with the current state
 */
static void __update_state() {
  uint16_t value;
#ifdef CONFIG_INVERT_SCREEN
  if (m_inverted && m_mirrored) {
    value = ILI9225_ENTRY_MODE_INVERTED_MIRRORED;
  } else if (m_inverted && !m_mirrored) {
    value = ILI9225_ENTRY_MODE_INVERTED;
  } else if (!m_inverted && m_mirrored) {
    value = ILI9225_ENTRY_MODE_NORMAL_MIRRORED;
  } else {
    value = ILI9225_ENTRY_MODE_NORMAL;
  }
#else
  if (m_inverted && m_mirrored) {
    value = ILI9225_ENTRY_MODE_NORMAL_MIRRORED;
  } else if (m_inverted && !m_mirrored) {
    value = ILI9225_ENTRY_MODE_NORMAL;
  } else if (!m_inverted && m_mirrored) {
    value = ILI9225_ENTRY_MODE_INVERTED_MIRRORED;
  } else {
    value = ILI9225_ENTRY_MODE_INVERTED;
  }
#endif

  xSemaphoreTake(m_mutex, portMAX_DELAY);
  __register(ILI9225_ENTRY_MODE, m_rgb_order | value);
  xSemaphoreGive(m_mutex);
}

void drv_ili9225_backlight_set(uint8_t brightness) {
  led_set_raw(31, brightness, brightness, brightness);
  led_show();
  ESP_LOGD(TAG, "Setting Screen LED to %d", brightness);
}

/**
 * @brief Initialize the LCD driver
 */
void drv_ili9225_init() {
  m_mutex = xSemaphoreCreateMutex();

  esp_err_t ret;
  spi_bus_config_t buscfg = {.miso_io_num = -1,
                             .mosi_io_num = LCD_PIN_MOSI,
                             .sclk_io_num = LCD_PIN_CLK,
                             .quadwp_io_num = -1,
                             .quadhd_io_num = -1};

  spi_device_interface_config_t devcfg = {
      .clock_speed_hz = 40000000,  // Clock out at 40 MHz
      .mode = 0,                   // SPI mode 0
      .spics_io_num = LCD_PIN_CS,  // CS pin
      .queue_size = 7,  // We want to be able to queue 7 transactions at a time
      .pre_cb = __spi_pre_callback,  // Specify pre-transfer callback to handle
                                     // D/C line
  };

  // Initialize the SPI bus
  ret = spi_bus_initialize(VSPI_HOST, &buscfg, 2);
  assert(ret == ESP_OK);
  // Attach the LCD to the SPI bus
  ret = spi_bus_add_device(VSPI_HOST, &devcfg, &m_spi);
  assert(ret == ESP_OK);

  // Initialize non-SPI GPIOs
  gpio_set_direction(LCD_PIN_DC, GPIO_MODE_OUTPUT);
  gpio_set_direction(LCD_PIN_RST, GPIO_MODE_OUTPUT);

  // Reset the display
  gpio_set_level(LCD_PIN_RST, 0);
  DELAY(100);
  gpio_set_level(LCD_PIN_RST, 1);
  DELAY(100);

  /** Start initial sequence **/
  __register(ILI9225_DRIVER_OUTPUT_CTRL, 0x011C);
  __register(ILI9225_LCD_AC_DRIVING_CTRL, 0x0100);

#ifdef CONFIG_INVERT_SCREEN
  __register(ILI9225_ENTRY_MODE, m_rgb_order | ILI9225_ENTRY_MODE_NORMAL);
#else
  __register(ILI9225_ENTRY_MODE, m_rgb_order | ILI9225_ENTRY_MODE_INVERTED);
#endif
  __register(ILI9225_BLANK_PERIOD_CTRL1, 0x0808);
  __register(ILI9225_INTERFACE_CTRL, 0x0000);
  __register(ILI9225_OSC_CTRL, 0x0801);
  __register(ILI9225_RAM_ADDR_SET1, 0x0000);
  __register(ILI9225_RAM_ADDR_SET2, 0x0000);

  /** Power on sequence **/
  DELAY(50);
  __register(ILI9225_POWER_CTRL1, 0x0A00);
  __register(ILI9225_POWER_CTRL2, 0x1038);
  DELAY(50);
  __register(ILI9225_POWER_CTRL3, 0x1121);
  __register(ILI9225_POWER_CTRL4, 0x0066);
  __register(ILI9225_POWER_CTRL5, 0x5F60);

  /** Set GRAM area **/
  __register(ILI9225_GATE_SCAN_CTRL, 0x0000);
  __register(ILI9225_VERTICAL_SCROLL_CTRL1, 0x00DB);
  __register(ILI9225_VERTICAL_SCROLL_CTRL2, 0x0000);
  __register(ILI9225_VERTICAL_SCROLL_CTRL3, 0x0000);
  __register(ILI9225_PARTIAL_DRIVING_POS1, 0x00DB);
  __register(ILI9225_PARTIAL_DRIVING_POS2, 0x0000);
  __register(ILI9225_HORIZONTAL_WINDOW_ADDR1, 0x00AF);
  __register(ILI9225_HORIZONTAL_WINDOW_ADDR2, 0x0000);
  __register(ILI9225_VERTICAL_WINDOW_ADDR1, 0x00DB);
  __register(ILI9225_VERTICAL_WINDOW_ADDR2, 0x0000);

  /** Set Gamma Curve **/
  __register(ILI9225_GAMMA_CTRL1, 0x0400);
  __register(ILI9225_GAMMA_CTRL2, 0x060B);
  __register(ILI9225_GAMMA_CTRL3, 0x0C0A);
  __register(ILI9225_GAMMA_CTRL4, 0x0105);
  __register(ILI9225_GAMMA_CTRL5, 0x0A0C);
  __register(ILI9225_GAMMA_CTRL6, 0x0B06);
  __register(ILI9225_GAMMA_CTRL7, 0x0004);
  __register(ILI9225_GAMMA_CTRL8, 0x0501);
  __register(ILI9225_GAMMA_CTRL9, 0x0E00);
  __register(ILI9225_GAMMA_CTRL10, 0x000E);
  DELAY(50);
  __register(ILI9225_DISP_CTRL1, 0x1017);

  ESP_LOGD(TAG, "ILI9225 Initialized");
}

/**
 * Push many colors to the display
 */
inline void drv_ili9225_push_colors(uint8_t* p_colors, int32_t size) {
  esp_err_t ret;
  int32_t chunk_size = MIN(SPI_MAX_DMA_LEN, size);

  while (size > 0) {
    //		spi_transaction_t *p_spi_tx = (spi_transaction_t *)
    // heap_caps_malloc(sizeof(spi_transaction_t), MALLOC_CAP_DMA);
    // spi_transaction_t *p_spi_tx = (spi_transaction_t *)
    // malloc(sizeof(spi_transaction_t__spi_pre_callback));
    spi_transaction_t tx;
    memset(&tx, 0, sizeof(spi_transaction_t));  // Zero out the transaction
    tx.length = chunk_size * 8;
    tx.tx_buffer = p_colors;  // The data is the cmd itself
    tx.user = (void*)1;       // D/C needs to be set high

    xSemaphoreTake(m_mutex, portMAX_DELAY);
    ret = spi_device_transmit(m_spi, &tx);  // Transmit!
    switch (ret) {
      case ESP_OK:
        break;
      case ESP_ERR_NO_MEM:
        ESP_LOGE(TAG,
                 "%s unable to send SPI data, reason: ESP_ERR_NO_MEM. Free DMA "
                 "space: %d bytes",
                 __func__, heap_caps_get_free_size(MALLOC_CAP_DMA));
        break;
      default:
        ESP_LOGE(TAG, "%s unable to send SPI data, reason: 0x%02X", __func__,
                 ret);
        break;
    }
    xSemaphoreGive(m_mutex);

    p_colors += chunk_size;
    size -= chunk_size;
    chunk_size = MIN(SPI_MAX_DMA_LEN, size);
  }

  //	spi_device_get_trans_result()
}

/**
 * @brief Get the current inversion state of the screen. This properly handles
 * the differences between Pickle Rick, Jerry Smith, and Mr Meeseeks hardware
 * revisions
 * @return True if inverted.
 */
bool drv_ili9225_is_inverted() {
  return m_inverted;
}

void drv_ili9225_invert(bool inverted) {
  m_inverted = inverted;
  __update_state();
}

/**
 * @brief Mirror the display
 */
void drv_ili9225_mirror(bool mirror) {
  m_mirrored = mirror;
  __update_state();
}

/**
 * @brief Sets if RGB mode should be used.
 * @param rgb : true for RGB false for BGR
 */
void drv_ili9225_rgb_mode(bool rgb) {
  if (rgb) {
    m_rgb_order = ILI9225_ENTRY_MODE_RGB;
  } else {
    m_rgb_order = ILI9225_ENTRY_MODE_BGR;
  }
  __update_state();
}

void drv_ili9225_set_addr(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
  xSemaphoreTake(m_mutex, portMAX_DELAY);

  // Column addr set
  __register(ILI9225_HORIZONTAL_WINDOW_ADDR1, y1);  // X End
  __register(ILI9225_HORIZONTAL_WINDOW_ADDR2, y0);  // X Start

  // Row addr set
  __register(ILI9225_VERTICAL_WINDOW_ADDR1, x1);  // Y End
  __register(ILI9225_VERTICAL_WINDOW_ADDR2, x0);  // Y Start

  if (m_inverted) {
    __register(ILI9225_RAM_ADDR_SET1,
               y1);  // NOTE: This should be y0 if the screen is inverted
    __register(ILI9225_RAM_ADDR_SET2, x0);
  } else {
    __register(ILI9225_RAM_ADDR_SET1, y0);
    __register(ILI9225_RAM_ADDR_SET2, x1);
  }

  __cmd(ILI9225_GRAM_DATA_REG);

  xSemaphoreGive(m_mutex);
}