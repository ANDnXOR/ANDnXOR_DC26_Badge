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

const static char* TAG = "MRMEESEEKS::WiFi";

/* FreeRTOS event group to signal when we are connected & ready to make a
 * request */
static EventGroupHandle_t __wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about
 * one event - are we connected to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

static IRAM_ATTR esp_err_t __event_handler(void* ctx, system_event_t* event) {
  switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
      ESP_LOGD(TAG, "%sSYSTEM_EVENT_STA_START", LOG_RED);
      esp_wifi_connect();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      ESP_LOGD(TAG, "%sSYSTEM_EVENT_STA_GOT_IP", LOG_RED);
      xEventGroupSetBits(__wifi_event_group, CONNECTED_BIT);
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      ESP_LOGD(TAG,
               "%sSYSTEM_EVENT_STA_DISCONNECTED, reconnecting. Is this what we "
               "want?",
               LOG_RED);
      /* This is a workaround as ESP32 WiFi libs don't currently
       * auto-reassociate. */
      // esp_wifi_connect();
      xEventGroupClearBits(__wifi_event_group, CONNECTED_BIT);
      break;
    default:
      break;
  }
  return ESP_OK;
}

/**
 * @brief Initialize the wifi module but don't connect
 */
inline void wifi_initialize() {
  __wifi_event_group = xEventGroupCreate();

  tcpip_adapter_init();

  // ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
  if (esp_event_loop_init(__event_handler, NULL) == ESP_FAIL) {
    ESP_LOGD(TAG, "Event Loop not created , may have been created before \n");
  }
}

/**
 * @brief Get connected state of wifi
 * @return true if currently connected
 */
inline bool wifi_is_connected() {
  EventBits_t bits = xEventGroupGetBits(__wifi_event_group);
  return (bits & CONNECTED_BIT) > 0;
}

/**
 * Generate a soft AP with a given SSID
 * @param ssid : Name of the AP to run
 */
void IRAM_ATTR wifi_soft_ap(const char* ssid) {
  util_heap_stats_dump();
  wifi_initialize();
  util_heap_stats_dump();

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_LOGD(TAG, "%s %sStarting Soft AP '%s'", __func__, LOG_CYAN, ssid);
  ESP_ERROR_CHECK(esp_wifi_start());

  ESP_LOGD(TAG, "%s %sWifi started, soft AP time", __func__, LOG_CYAN);

  //	int bits = xEventGroupWaitBits(__wifi_event_group, CONNECTED_BIT, 0, 1,
  // 0);

  wifi_config_t wifi_config = {0};
  wifi_config.ap.channel = util_random(0, 13);

  strlcpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
  //	ESP_ERROR_CHECK(esp_wifi_connect());

  ESP_LOGD(TAG, "%s %sAP Started", __func__, LOG_CYAN);

  xEventGroupWaitBits(__wifi_event_group, CONNECTED_BIT, 0, 1,
                      CONFIG_WIFI_CONNECT_TIMEOUT / portTICK_RATE_MS);
}

/**
 * Stop the soft AP
 */
void IRAM_ATTR wifi_soft_ap_stop() {
  esp_wifi_disconnect();
  while (wifi_is_connected()) {
    DELAY(10);
  }
  esp_wifi_stop();
  esp_wifi_deinit();
}

/**
 * @brief Start the wifi module and connect to the given SSID
 * @param wc : Wifi config to use for the connection
 */
void IRAM_ATTR wifi_start(wifi_config_t wc) {
  gfx_stop();

  util_heap_stats_dump();
  wifi_initialize();
  util_heap_stats_dump();

  // Initialize the radio with blank default config pre-startup
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));
  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
  ESP_ERROR_CHECK(esp_wifi_set_mode(
      WIFI_MODE_STA));  // WIFI_MODE_NULL was removed fro ESP SDK Support

  ESP_LOGD(TAG, "Starting wifi driver");
  util_heap_stats_dump();
  ESP_ERROR_CHECK(esp_wifi_start());
  ESP_LOGD(TAG, "WiFi Start sequence completed...\n");

  // Now give it the config of what u want to connect to
  wifi_config_t wifi_config = wc;
  ESP_LOGD(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
  ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
  ESP_LOGD(TAG, "WiFi Mode has been set... \n");
  ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
  ESP_LOGD(TAG, "WiFi Configuration Loaded... \n");
  ESP_ERROR_CHECK(esp_wifi_connect());
  ESP_LOGD(TAG, "WiFi Connecting to AP... \n");

  // TODO This TimeOut is not working
  xEventGroupWaitBits(__wifi_event_group, CONNECTED_BIT, 0, 1,
                      CONFIG_WIFI_CONNECT_TIMEOUT / portTICK_PERIOD_MS);
}

/**
 * @brief Stop the wifi module and return badge to previous state
 */
void IRAM_ATTR wifi_stop() {
  // Disconnect from the AP
  esp_wifi_disconnect();

  while (wifi_is_connected()) {
    DELAY(10);
  }

  // Stop the Wi-Fi driver
  esp_wifi_stop();

  // Workaround a race condition between stop and de-initialization
  // Mutex, i dont need no stinkin mutex...
  DELAY(100);

  // Clean Up Event States
  esp_wifi_deinit();

  // TODO CLEANUP
  // There's no reason we should delete the thread when stopping the radio
  // vEventGroupDelete(__wifi_event_group);

  // TODO CLEANUP
  // Unload the Wi-Fi driver
  // There's no reason we should unload the driver when stopping the radio
  // esp_wifi_deinit();
}
