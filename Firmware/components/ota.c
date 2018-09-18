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

#define OTA_VERSION_FILE_MAX_SIZE 100

const static char* TAG = "MRMEESEEKS::OTA";
static volatile uint32_t last_ota = 0;
static volatile bool ota_running = false;
static uint32_t m_next_ota_screen_time = 0;

typedef struct {
  char url[256];
  char path[256];
  FILE* file;
  size_t bytes_written;
} ota_file_t;

/**
 * @brief Callback for when n bytes are downloaded from server
 * @param req : Request structure for the current file
 * @param data : The data downloaded
 * @param len : The length of the data downloaded
 * @return 0 if okay
 */
static int __download_file_callback(request_t* req, char* data, int len) {
  ota_file_t* p_ota = (ota_file_t*)req->context;

  // Give a status
  uint32_t now = time_manager_now_sec();
  if (now >= m_next_ota_screen_time) {
    gfx_font_set(font_large);
    gfx_cursor_set((cursor_coord_t){0, 76});
    gfx_color_set(COLOR_GREEN);
    gfx_fill_screen(COLOR_BLACK);

    float pct = ((float)p_ota->bytes_written / 1500000.0) * 100;
    char text[64];
    sprintf(text, "OTA: %.1f%%", pct);
    gfx_print(text);
    gfx_push_screen_buffer();
    m_next_ota_screen_time = now + 2;
  }

  if (p_ota->file != NULL) {
    fwrite(data, 1, len, p_ota->file);
    p_ota->bytes_written += len;
    // ESP_LOGD(TAG, "Downloaded %d bytes", p_ota->bytes_written);
  } else {
    ESP_LOGE(TAG, "Could not open %s for writing.", p_ota->path);
  }
  return 0;
}

static void __download_file(ota_file_t* p_ota) {
  ESP_LOGI(TAG, "Downloading file %s to %s", p_ota->url, p_ota->path);

  // Delete file if it exists
  if (util_file_exists(p_ota->path)) {
    ESP_LOGD(TAG, "%s exists, deleting", p_ota->path);
    unlink(p_ota->path);
  }

  p_ota->file = fopen(p_ota->path, "w");
  p_ota->bytes_written = 0;

  request_t* req;
  req = req_new(p_ota->url);
  req_setopt(req, REQ_FUNC_DOWNLOAD_CB, __download_file_callback);
  req_setopt(req, REQ_SET_HEADER, "User-Agent: " USER_AGENT);
  req->context = p_ota;

  util_heap_stats_dump();
  req_perform(req);
  req_clean(req);

  fclose(p_ota->file);
}

/**
 * @brief Check the version file on the SD card
 * @return True if the version file on the SD card is newer and we need to
 * download an update
 */
static bool __version_file_check() {
  // If version file is not there, we can't check and shouldn't proceed
  if (!util_file_exists(OTA_FIRMWARE_VERSION_FILE)) {
    ESP_LOGE(TAG, "Version file is missing.");
    return false;
  }

  // If version file size is odd, don't open and don't proceed
  uint32_t size = util_file_size(OTA_FIRMWARE_VERSION_FILE);
  if (size == 0 || size > OTA_VERSION_FILE_MAX_SIZE) {
    ESP_LOGE(TAG, "Version file is a bit odd.");
    return false;
  }

  FILE* file = fopen(OTA_FIRMWARE_VERSION_FILE, "r");
  // If version file can't be opened, we can't check and shouldn't proceed
  if (!file) {
    ESP_LOGE(TAG, "Could not open version file :(");
    return false;
  }

  char buffer[OTA_VERSION_FILE_MAX_SIZE];

  // We got thss far, version file is able to be opened and sorta close to
  // expected size
  size_t bytes_read = fread(buffer, 1, size, file);
  buffer[bytes_read] = 0;  // ensure null terminated

  // Parse new line and get version numbers
  char* buffer_nl = strchr(buffer, '\n');
  uint16_t version = strtol(buffer, &buffer_nl, 10);
  char* version_human = buffer_nl + 1;
  char* new_line = strchr(version_human, '\n');
  if (new_line != NULL) {
    *new_line = 0;
  }

  ESP_LOGD(TAG,
           "Versiono file parsed. Version %d Human version '%s' My Version %d",
           version, version_human, VERSION_INT);

  fclose(file);

  return version > VERSION_INT;
}

/**
 * @brief Download the latest OTA
 * @param parameters : Task parameters, ignored
 */
void ota_download_task(void* parameters) {
  if (ota_running) {
    ESP_LOGD(TAG, "OTA is running");
    vTaskDelete(NULL);
    return;
  }

  ota_running = true;

  // Rate limit OTA attempts
  if (time_manager_now_sec() <= last_ota + CONFIG_OTA_RATE_LIMIT_SECONDS) {
    ESP_LOGD(TAG, "%sRate limiting OTA. now=%d next=%d", LOG_YELLOW,
             time_manager_now_sec(),
             (last_ota + CONFIG_OTA_RATE_LIMIT_SECONDS));
    ota_running = false;
    vTaskDelete(NULL);
    return;
  }

  wifi_config_t wifi_config = {
      .sta =
          {
              .ssid = CONFIG_WIFI_SSID,
              .password = CONFIG_WIFI_PASSWORD,
          },
  };

  ESP_LOGD(TAG, "Connecting to '%s'", wifi_config.sta.ssid);
  wifi_start(wifi_config);

  if (wifi_is_connected()) {
    ESP_LOGD(TAG, "Connected to AP");

    // Download version file
    ota_file_t ota_version;
    sprintf(ota_version.url, OTA_FIRMWARE_VERSION_URL);
    sprintf(ota_version.path, OTA_FIRMWARE_VERSION_FILE);
    __download_file(&ota_version);

    // Check version, proceed if newer
    if (!__version_file_check()) {
      ESP_LOGD(TAG, "No new OTA available");
      goto ota_done;
    }

    // Version check passed if we got this far
    gfx_stop();

    // Download bin file
    ESP_LOGD(TAG, "Downloading firmware bin file");
    ota_file_t ota_bin;
    sprintf(ota_bin.url, OTA_FIRMWARE_BIN_URL);
    sprintf(ota_bin.path, OTA_FIRMWARE_BIN_FILE);
    __download_file(&ota_bin);
    ESP_LOGD(TAG, "Done.");

    // Download sha256 file
    ESP_LOGD(TAG, "Downloading firmware hash file");
    ota_file_t ota_sha256;
    sprintf(ota_sha256.url, OTA_FIRMWARE_SHA256_URL);
    sprintf(ota_sha256.path, OTA_FIRMWARE_SHA256_FILE);
    __download_file(&ota_sha256);
    ESP_LOGD(TAG, "Done.");

    // Make sure bin file size is close to what it should be
    if (util_file_size(OTA_FIRMWARE_BIN_FILE) > 1400000 &&
        util_file_size(OTA_FIRMWARE_SHA256_FILE) == 64) {
      ESP_LOGI(TAG, "Firmware file valid, rebooting");
      ota_reboot_and_flash();
    }
    // SHA256 failed, delete files and try again some other time
    else {
      ESP_LOGE(TAG, "Invalid firmware downloaded");
      unlink(OTA_FIRMWARE_BIN_FILE);
      unlink(OTA_FIRMWARE_VERSION_FILE);
      unlink(OTA_FIRMWARE_SHA256_FILE);
    }

  ota_done:
    ESP_LOGD(TAG, "OTA Done");
    // Remember last OTA
    last_ota = time_manager_now_sec();
  }

  wifi_stop();

  ota_running = false;
  vTaskDelete(NULL);
}

/**
 * @brief Reboot the badge into the boot stub to force an update
 */
void ota_reboot_and_flash() {
  esp_err_t err;

  // Find the factory (boot_stub) partition
  esp_partition_iterator_t iterator = esp_partition_find(
      ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_APP_FACTORY, NULL);

  if (iterator == NULL) {
    ESP_LOGE(TAG, "boot_stub partition missing. Cannot OTA.");
    return;
  }

  // Get it (if it exists)
  const esp_partition_t* p_partition = esp_partition_get(iterator);

  if (p_partition == NULL) {
    ESP_LOGE(TAG, "boot_stub partition missing. Cannot OTA.");
    return;
  }

  // Set the factory boot_stub partition as the boot
  err = esp_ota_set_boot_partition(p_partition);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Unable to set boot partition to factory.");
    return;
  }

  ESP_LOGI(TAG, "Boot partition set to factory. Restarting.");
  esp_restart();
}