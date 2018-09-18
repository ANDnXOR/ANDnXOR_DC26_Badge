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

#define BENCHMARK_FILENAME "/sdcard/large_file.dat"
#define BENCHMARK_BUFFER_SIZE 20000
#define BENCHMARK_VBUF_SIZE (BENCHMARK_BUFFER_SIZE * 4)

static const char* TAG = "MRMEESEEKS::SD";
static const char* DC25_HASH =
    "5acbcc35c2d9348b464a34ec80481446553194f3a4b7765bccd101434e0f3e7b";
static xQueueHandle m_evt_queue = NULL;
static volatile bool m_mounted = false;

static void __mount();
static void __unmount();

/**
 * @brief Test for specific file(s) and hashes on the SD card to see if it is a
 * DC25 card and unlock
 */
static void __dc25_test() {
  //Only allow this to be done once
  if ((state_unlock_get() & UNLOCK_DC25_SD) > 0) {
    return;
  }

  // Test for specific files on the SD Card
  if (util_file_exists("/sdcard/BG.RAW") &&
      util_file_exists("/sdcard/INTRO.RAW")) {
    char hash[SHA256_DIGEST_SIZE_BYTES + 1];

    // Check it's hash
    util_file_sha256("/sdcard/BG.RAW", hash);
    if (strcmp(hash, DC25_HASH) == 0) {
      ESP_LOGI(TAG, "DC25 SD Card Inserted! Unlocking.");
      state_unlock_set(state_unlock_get() | UNLOCK_DC25_SD);
      state_save_indicate();
      ui_popup_info("Badger Badger Badger... New bling available. Put DC26 back first!");
    }
  }
}

static void __sd_cd_handler(void* arg) {
  uint32_t mask = 0;
  xQueueSendFromISR(m_evt_queue, &mask, NULL);
}

static void __task_cd_handler(void* arg) {
  ESP_LOGD(TAG, "SD Card Detect Task Starting");
  uint32_t mask;
  for (;;) {
    if (xQueueReceive(m_evt_queue, &mask, portMAX_DELAY)) {
      uint8_t level = gpio_get_level(SD_CARD_DETECT);
      ESP_LOGD(TAG, "SD card detect changed");

      // Basic software debounce of SD card. I guess this works? SD card is
      // spring loaded.
      DELAY(500);
      if (level == gpio_get_level(SD_CARD_DETECT)) {
        switch (level) {
          case 0:
            // Card missing
            __unmount();
            break;
          case 1:
            // Card inserted
            __mount();
            break;
        }
      }
    }
  }
}

/**
 * @brief Check for availability of OTA files and reboot into boot_stub if
 * necessary
 */
static void __ota_check() {
  if (util_file_exists(OTA_FIRMWARE_BIN_FILE) &&
      util_file_exists(OTA_FIRMWARE_SHA256_FILE)) {
    ESP_LOGD(TAG, "OTA files located, attempting to OTA.");
    ota_reboot_and_flash();
  } else {
    ESP_LOGD(TAG, "No OTA files found, continuing");
  }
}

/**
 * Mount the SD card
 */
static void __mount() {
  if (m_mounted) {
    return;
  }

  ESP_LOGD(TAG, "Mounting");

  sdmmc_host_t host = SDMMC_HOST_DEFAULT();

  // This initializes the slot without card detect (CD) and write protect (WP)
  // signals.
  // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these
  // signals.
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  // slot_config.gpio_cd = 39;

  // GPIOs 15, 2, 4, 12, 13 should have external 10k pull-ups.
  // Internal pull-ups are not sufficient. However, enabling internal pull-ups
  // does make a difference some boards, so we do that here.
  gpio_set_pull_mode(15,
                     GPIO_PULLUP_ONLY);  // CMD, needed in 4- and 1- line modes
  gpio_set_pull_mode(2, GPIO_PULLUP_ONLY);  // D0, needed in 4- and 1-line modes
  gpio_set_pull_mode(4, GPIO_PULLUP_ONLY);  // D1, needed in 4-line mode only
  gpio_set_pull_mode(12, GPIO_PULLUP_ONLY);  // D2, needed in 4-line mode only
  gpio_set_pull_mode(13,
                     GPIO_PULLUP_ONLY);  // D3, needed in 4- and 1-line modes

  gpio_set_pull_mode(0, GPIO_PULLUP_ONLY);  // Not for SD but on Jerry Smith and
                                            // later 0 is tied to 2

#ifdef CONFIG_SD_CARD_FAILSAFE
  host.flags = SDMMC_HOST_FLAG_1BIT; 
#else
  host.flags = SDMMC_HOST_FLAG_4BIT;
#endif

  // Mount config
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
      .format_if_mount_failed = false, .max_files = 10};

  // Actually attempt to mount the card
  sdmmc_card_t* card;
  esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config,
                                          &mount_config, &card);

  if (ret != ESP_OK) {
    if (ret == ESP_FAIL) {
      ESP_LOGE(TAG,
               "Failed to mount filesystem. "
               "If you want the card to be formatted, set "
               "format_if_mount_failed = true.");
    } else {
      ESP_LOGE(TAG,
               "Failed to initialize the card (%d). "
               "Make sure SD card lines have pull-up resistors in place.",
               ret);
    }
    post_state_get()->sd_card_peripheral = false;
    return;
  }

#ifdef CONFIG_SD_CARD_FAILSAFE
  sdmmc_host_set_card_clk(card->host.slot, 20000);
#else
  sdmmc_host_set_card_clk(card->host.slot, 40000);
#endif

  // Card has been initialized, print its properties
  sdmmc_card_print_info(stdout, card);
  post_state_get()->sd_card_peripheral = true;

  ESP_LOGI(TAG, "SD Card Mounted");
  m_mounted = true;

  // Check for OTA files and reboot if necessary
  __ota_check();

  // Test for DC25 SD card :)
  __dc25_test();
}

/**
 * Unmount the SD card
 */
static void __unmount() {
  if (!m_mounted) {
    return;
  }

  esp_vfs_fat_sdmmc_unmount();
  m_mounted = false;
  ESP_LOGI(TAG, "SD Card Unmounted");
}

void drv_sd_benchmark() {
  FILE* f = fopen(BENCHMARK_FILENAME, "r");
  size_t read_bytes = 0;
  //	void *buffer = heap_caps_malloc(BENCHMARK_BUFFER_SIZE, MALLOC_CAP_DMA);
  void* buffer = malloc(BENCHMARK_BUFFER_SIZE);

  // Get the size of the benchmark file
  uint32_t fsize = util_file_size(BENCHMARK_FILENAME);
  if (fsize == 0) {
    ESP_LOGE(TAG, "Could not stat %s.", BENCHMARK_FILENAME);
    return;
  }

  fsize = MIN(fsize, 30 * 320 * 240 * 2);

  // Allocate some buffer
  char* buf = (char*)heap_caps_malloc(BENCHMARK_VBUF_SIZE, MALLOC_CAP_DMA);
  setvbuf(f, buf, _IOFBF, BENCHMARK_VBUF_SIZE);
  if (f == NULL) {
    ESP_LOGE(TAG, "Could not open %s.", BENCHMARK_FILENAME);
    free(buf);
    return;
  }

  ESP_LOGD(TAG, "Free DMA space: %d bytes",
           heap_caps_get_free_size(MALLOC_CAP_DMA));

  ESP_LOGD(TAG, "Benchmarking SD with %d bytes file.", fsize);
  uint32_t start = MILLIS();
  while (read_bytes < fsize) {
    read_bytes += fread(buffer, 1, BENCHMARK_BUFFER_SIZE, f);
    //		ESP_LOGI(TAG, "Read %d bytes", read_bytes);
  }
  uint32_t delta = MILLIS() - start;

  fclose(f);
  free(buf);

  uint32_t kbps = (read_bytes * 1000L) / delta / 1024L;

  ESP_LOGD(
      TAG,
      "SD Benchmark Complete. Read %d bytes in %d ms. Transfer rate: %d kBps",
      read_bytes, delta, kbps);
}

void drv_sd_init() {
  gpio_config_t io_conf;

  // Interrupt on falling edge
  io_conf.intr_type = GPIO_PIN_INTR_ANYEDGE;
  // bit mask of the pins
  io_conf.pin_bit_mask = GPIO_SEL_39;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = 0;
  io_conf.pull_down_en = 0;
  gpio_config(&io_conf);

#ifndef CONFIG_SD_CARD_CD_OVERRIDE
  m_evt_queue = xQueueCreate(1, sizeof(uint32_t));
  xTaskCreatePinnedToCore(__task_cd_handler, "SD Card Detect", 4000, NULL,
                          TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);

  // Hook ISR for card detect
  gpio_isr_handler_add((gpio_num_t)SD_CARD_DETECT, __sd_cd_handler, NULL);
#else
  __mount();
#endif
}

bool drv_sd_mounted() {
  return m_mounted;
}
