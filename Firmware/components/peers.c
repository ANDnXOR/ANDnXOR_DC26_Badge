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

#define PEERS_FILE "/sdcard/peers.dat"

#define TPL_MAP \
  "A(S(Uc#uu))" /* Mapping string for TPL to pack and unpack peers data */

const static char* TAG = "MRMEESEEKS::Peers";

peer_t* p_peers = NULL;
static xQueueHandle m_hello_evt_queue = NULL;

static void __hello_task(void* params) {
  peer_t peer;
  while (1) {
    // Delay initial startup and between future "hellos" to effectively rate
    // limit
    DELAY(CONFIG_PEERS_HELLO_RATE_LIMIT);

    // Clear the queue
    xQueueReceive(m_hello_evt_queue, &peer, 1000 / portTICK_PERIOD_MS);

    // Wait for something to come into the queue and the SD card to be mounted
    if (xQueueReceive(m_hello_evt_queue, &peer, portMAX_DELAY) &&
        drv_sd_mounted()) {
      ESP_LOGD(TAG, "HELLO %s!", peer.name);
      char* raw_file = NULL;

      // ESP_LOGD(TAG, "%sSaying Hello to 0x%02x", LOG_YELLOW, peer.company_id);

      char hello_str[256];
      // Check for rabies
      if (peer.magic == BLE_MAGIC_RABIES) {
        raw_file = "/sdcard/random/fry_crazy.raw";
        for (uint8_t i = 0; i < 8; i++) {
          hello_str[i] = util_random(INPUT_CHAR_MIN, INPUT_CHAR_MAX);
        }
        hello_str[8] = 0;
      } else {
        switch (peer.company_id) {
          case PEERS_COMPANY_ID_CPV:
            raw_file = "/sdcard/gfx/cpv.raw";
            sprintf(hello_str, "Hello CPV!");
            break;
          case PEERS_COMPANY_ID_DC801:
            raw_file = "/sdcard/gfx/dc801.raw";
            sprintf(hello_str, "Hello DC801!");
            break;
          case PEERS_COMPANY_ID_JOCO:
            raw_file = "/sdcard/gfx/joco.raw";
            sprintf(hello_str, "Hello %s!", peer.name);
            break;
          case PEERS_COMPANY_ID_ANDNXOR:
            raw_file = "/sdcard/bling/hi1.raw";
            sprintf(hello_str, "Hello %s!", peer.name);
            break;
          case PEERS_COMPANY_ID_BLINKYBLING:
            raw_file = "/sdcard/bling/hiblinkybling.raw";
            sprintf(hello_str, "Hello Mr Blinky Bling!");
            break;
          case PEERS_COMPANY_ID_DCFURS:
            raw_file = "/sdcard/bling/hifurs.raw";
            sprintf(hello_str, "Hello DEFCON Furs!");
            break;
          case PEERS_COMPANY_ID_TRANS_IONOSPHERIC1:
          case PEERS_COMPANY_ID_TRANS_IONOSPHERIC2:
            raw_file = "/sdcard/gfx/ionospheric.raw";
            sprintf(hello_str, "Hello Trans Ionospheric!");
            break;
          default:
            raw_file = "/sdcard/bling/hi1.raw";
            sprintf(hello_str, "Hello!");
            break;
        }
      }

      ui_interrupt(hello_str, raw_file, 10);
    }
  }
}

static int __timestamp_sort(peer_t* a, peer_t* b) {
  return a->timestamp - b->timestamp;
}

uint32_t peers_count() {
  return HASH_COUNT(p_peers);
}

/**
 * @brief Dump peers to console
 */
void peers_dump() {
  uint32_t now = time_manager_now_sec();
  peer_t* s;

  uint32_t total = 0;
  uint32_t counter = 0;
  for (s = p_peers; s != NULL; s = s->hh.next) {
    total++;  // count all peers

    // Only show peers from the last 2 minutes
    if (s->timestamp > (now - 120) && s->timestamp <= now) {
      // First peer print out header
      if (counter == 0) {
        printf("  ====================PEERS=====================\n");
        printf("  #  Name\t\tLast Seen (sec)\tAddress\n");
      }

      printf("  %u: %-10s\t%lu\t\t0x%06llx\n", counter++, s->name,
             now - s->timestamp, s->address);
    }
  }
  if (counter == 0) {
    printf("  No peers nearby right now :(\n");
  } else {
    printf("  ====================PEERS=====================\n");
  }
  printf("  This badge has seen %d peers in its lifetime.\n\n", total);
}

peer_t* peers_get_by_address(uint64_t address) {
  peer_t* peer = NULL;
  HASH_FIND_INT(p_peers, &address, peer);
  return peer;
}

peer_t* peers_hashtable_get() {
  return p_peers;
}

// void peers_hello() {
//   gfx_play_raw_file("/sdcard/bling/hi1.raw", 0, 0, LCD_WIDTH, LCD_HEIGHT,
//   NULL,
//                     false, NULL);
// }

bool peers_init() {
  // TODO: Load peers from file

  // Create the queue to store hellos that need to be displayed
  m_hello_evt_queue = xQueueCreate(1, sizeof(peer_t));

#ifndef CONFIG_BADGE_TYPE_STANDALONE
  // Start the hello task
  xTaskCreatePinnedToCore(__hello_task, "Hello", 4096, NULL, TASK_PRIORITY_HIGH,
                          NULL, APP_CPU_NUM);
#endif

  return true;
}

/**
 * Load peers from SD
 */
void peers_load() {
  // Make sure SD card is inserted
  if (!drv_sd_mounted()) {
    ESP_LOGE(TAG, "SD card not mounted, cannot save peers.");
    return;
  }

  uint32_t fsize = util_file_size(PEERS_FILE);
  if (fsize == 0) {
    ESP_LOGE(TAG, "Peers file is 0 bytes.");
    return;
  }

  // Open the file
  FILE* file = fopen(PEERS_FILE, "r");
  if (!file) {
    ESP_LOGE(TAG, "Unable to open '%s' for read. errno=%d", PEERS_FILE, errno);
    return;
  }

  // Get some heap to store serialized peers data
  void* buffer = util_heap_alloc_ext(fsize);

  // Read in the raw bytes
  if (fread(buffer, 1, fsize, file) == fsize) {
    // Unpack TPL data
    peer_t peer;
    tpl_node* tn;
    ESP_LOGD(TAG, "Mapping TPL");
    tn = tpl_map(TPL_MAP, &peer, STATE_NAME_LENGTH);
    ESP_LOGD(TAG, "%sLoading TPL from buffer", LOG_BLUE);

    // Read the buffer
    tpl_load(tn, TPL_MEM | TPL_PREALLOCD, buffer, fsize);

    // Unpack one peer at a time
    while (tpl_unpack(tn, 1) > 0) {
      peers_update_or_create(&peer, false);
    }

    tpl_free(tn);
  }

  // Cleanup
  free(buffer);
  fclose(file);
}

uint16_t peers_nearby_count() {
  uint16_t count = 0;
  peer_t* p;
  peer_t peer;
  uint32_t now = time_manager_now_sec();

  for (p = p_peers; p != NULL; p = p->hh.next) {
    peer = *p;

    // If nearby, add to counter
    if (peer.timestamp > (now - PEERS_NEARBY_TIME_SEC) ||
        (now < PEERS_NEARBY_TIME_SEC)) {
      count++;
    }
  }

  return count;
}

void IRAM_ATTR peers_save() {
  peer_t* s;

  // Make sure SD card is inserted
  if (!drv_sd_mounted()) {
    ESP_LOGE(TAG, "SD card not mounted, cannot save peers.");
    return;
  }

  ESP_LOGD(TAG, "Saving peers");

  // Walk through each peer
  s = p_peers;
  peer_t peer;
  tpl_node* tn = tpl_map(TPL_MAP, &peer, STATE_NAME_LENGTH);
  for (s = p_peers; s != NULL; s = s->hh.next) {
    peer = *s;
    tpl_pack(tn, 1);
  }

  // Write to memory
  size_t size;
  tpl_dump(tn, TPL_GETSIZE, &size);
  void* buffer = util_heap_alloc_ext(size);
  tpl_dump(tn, TPL_MEM | TPL_PREALLOCD, buffer, &size);
  tpl_free(tn);

  FILE* file = fopen(PEERS_FILE, "w");
  if (!file) {
    ESP_LOGE(TAG, "Could not open '%s' for write. errno=%d", PEERS_FILE, errno);
    return;
  }

  // Write to file
  fwrite(buffer, size, 1, file);
  // tpl_dump(tn, TPL_FD, fileno(file));
  fclose(file);

  // Open
  // esp_err_t err;
  // nvs_handle handle;
  // err = nvs_open(ANDNXOR_NVS_NAMESPACE, NVS_READWRITE, &handle);
  // if (err != ESP_OK) {
  //   ESP_LOGE(TAG, "Could not open NVS for badge state for saving.");
  //   free(buffer);
  //   return;
  // }

  // // Write to NVS
  // err = nvs_set_blob(handle, PEERS_NVS_KEY, buffer, size);
  // if (err != ESP_OK) {
  //   ESP_LOGE(TAG, "Could not write peers blob to NVS.");
  //   free(buffer);
  //   nvs_close(handle);
  //   return;
  // }

  // // Commit
  // ESP_LOGD(TAG, "%s Committing to NVS", __func__);
  // err = nvs_commit(handle);
  // if (err != ESP_OK) {
  //   ESP_LOGE(TAG, "Could not commit peers to NVS.");
  //   free(buffer);
  //   nvs_close(handle);
  //   return;
  // }

  // // Close
  // nvs_close(handle);

  ESP_LOGD(TAG, "Peers serialized. Size=%d", size);

  free(buffer);
}

/**
 * @brief Ensure local peer database is updated with a peer
 * @param other_peer : Peer to add or update the DB with
 * @param hello : Set to true if badge should try to say hello
 * @return The authoritative copy of the peer
 */
peer_t* peers_update_or_create(peer_t* other_peer, bool hello) {
  peer_t* peer;

  // Look for peer in the table
  HASH_FIND_INT(p_peers, &other_peer->address, peer);
  if (peer == NULL) {
    peer = (peer_t*)util_heap_alloc_ext(sizeof(peer_t));
    peer->address = other_peer->address;
    HASH_ADD_INT(p_peers, address, peer);
  }

  // At this point peer is guaranteed to be in the hashtable
  memcpy(peer->name, other_peer->name, STATE_NAME_LENGTH);
  peer->company_id = other_peer->company_id;
  peer->address = other_peer->address;
  peer->name[STATE_NAME_LENGTH] = 0;  // ensure null terminated
  peer->timestamp = other_peer->timestamp;
  peer->nonce = other_peer->nonce;

  HASH_SORT(p_peers, __timestamp_sort);
  while (HASH_COUNT(p_peers) > PEERS_MAX) {
    peer = p_peers;
    while (peer->hh.next != NULL) {
      peer = peer->hh.next;
    }
    HASH_DEL(p_peers, peer);
    ESP_LOGD(TAG, "Deleting peer %lld", peer->address);
    free(peer);
  }

  if (hello) {
    // ESP_LOGD(TAG, "%sQueueing %d for hello", LOG_YELLOW, peer->company_id);
    xQueueSendToFront(m_hello_evt_queue, peer, 1000 / portTICK_PERIOD_MS);
  }

  //	ESP_LOGD(TAG, "Added Peer %s[%d]", other_peer->name,
  //(int)other_peer->address);
  //	__peers_dump();

  // Return pointer to authoritative copy of the peer
  return peer;
}
