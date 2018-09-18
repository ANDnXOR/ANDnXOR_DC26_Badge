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

// LCI includes
#include "lulz_error.h"
#include "lexer.h"
#include "tokenizer.h"
#include "parser.h"
#include "interpreter.h"
#include "lolcode.h"

static const char* TAG = "lolcode";

// Store globals in a linked list
static lulz_global_t* p_globals_first = NULL;

static void __loading_task(void* parameters) {
  gfx_play_raw_file("/sdcard/bling/loading.raw", 0, 0, LCD_WIDTH, LCD_HEIGHT,
                    NULL, true, NULL);
  vTaskDelete(NULL);
}

static int __pipeline(char* buffer,
                      unsigned int length,
                      const char* fname,
                      bool show_loading) {
  LexemeList* lexemes = NULL;
  Token** tokens = NULL;
  MainNode* node = NULL;
  uint32_t start = MILLIS();

  static StaticTask_t task;
  if (show_loading) {
    util_task_create(__loading_task, "LULZCODE Loading", 4096, NULL,
                TASK_PRIORITY_MEDIUM, &task);
  }

  if (!(lexemes = lulz_scan_buffer(buffer, length, fname, false))) {
    // Stop any animation(namely loading)
    if (show_loading) {
      uint32_t now = MILLIS();
      if (now < (start + 500)) {
        ESP_LOGD(TAG, "Delaying %d", ((start + 500) - now));
        DELAY((start + 500) - now);
      }
      gfx_stop();
    }
    return 1;
  }

  YIELD();

  if (!(tokens = tokenizeLexemes(lexemes))) {
    deleteLexemeList(lexemes);
    // Stop any animation(namely loading)
    if (show_loading) {
      uint32_t now = MILLIS();
      if (now < (start + 500)) {
        ESP_LOGD(TAG, "Delaying %d", ((start + 500) - now));
        DELAY((start + 500) - now);
      }
      gfx_stop();
    }
    return 1;
  }

  YIELD();

  deleteLexemeList(lexemes);
  if (!(node = parseMainNode(tokens))) {
    deleteTokens(tokens);
    // Stop any animation(namely loading)
    if (show_loading) {
      uint32_t now = MILLIS();
      if (now < (start + 500)) {
        ESP_LOGD(TAG, "Delaying %d", ((start + 500) - now));
        DELAY((start + 500) - now);
      }
      gfx_stop();
    }
    return 1;
  }
  deleteTokens(tokens);

  YIELD();

  // Stop any animation(namely loading)
  if (show_loading) {
    uint32_t now = MILLIS();
    if (now < (start + 500)) {
      ESP_LOGD(TAG, "Delaying %d", ((start + 500) - now));
      DELAY((start + 500) - now);
    }
    gfx_stop();
  }

  // Make sure fonts are transparent
  gfx_bg_transparent(true);

  // Make sure we aren't clipping things
  gfx_cursor_area_reset();

  uint8_t result = interpretMainNode(node);
  deleteMainNode(node);

  // Reset cursor area to avoid clipping
  gfx_cursor_area_reset();
  return result;
}

bool lolcode_execute(const char* file_name, bool show_loading) {
  if (!drv_sd_mounted()) {
    char buffer[256];
    sprintf(buffer, "Unable to execute '%s', no SD card inserted.", file_name);
    ESP_LOGE(TAG, "%s", buffer);
    gfx_error(buffer);
    return false;
  }

  size_t before = xPortGetFreeHeapSize();
  FILE* ffd;

  // Read file that was just uploaded
  ffd = fopen(file_name, "r");
  if (ffd == NULL) {
    gfx_stop();
    ESP_LOGE(TAG, "Failed to open file for reading");
    return false;
  }

  char* lols = (char*)util_heap_alloc_ext(LULZ_MAX_FILE_SIZE);
  memset(lols, 0, LULZ_MAX_FILE_SIZE);
  int i = 0;
  while (i < LULZ_MAX_FILE_SIZE) {
    int c = fgetc(ffd);
    if (c == EOF) {
      break;
    }
    lols[i++] = c;
  }
  fclose(ffd);

  //	ESP_LOGD(TAG, "File
  //contents:\n-----------------\n%s\n-------------------\n", lols);
  util_heap_stats_dump();
  __pipeline(lols, strlen(lols), file_name, show_loading);

  // Dump the file
  vPortFree(lols);

  size_t after = xPortGetFreeHeapSize();
  size_t leak = before - after;
  util_heap_stats_dump();

  if (leak > 0) {
    ESP_LOGD(TAG, "%sLOLCODE Generated a memory leak! %d bytes", LOG_RED, leak);
  }

  return true;
}

char* lulz_global_get(char* key) {
  ESP_LOGD(TAG, "lulz_global_get('%s')", key);

  lulz_global_print();

  if (key == NULL) {
    return NULL;
  }

  // Walk the linked list looking for the global key
  lulz_global_t* ptr = p_globals_first;
  while (ptr != NULL) {
    if (strcmp(key, ptr->key) == 0) {
      return ptr->value;
    }
    ptr = ptr->p_next;
  }

  return NULL;
}

void lulz_global_print() {
  ESP_LOGD(TAG, "LULZCODE Globals");
  ESP_LOGD(TAG, "===============");
  lulz_global_t* ptr = p_globals_first;
  uint8_t i = 0;
  while (ptr != NULL) {
    ESP_LOGD(TAG, "%d) Global '%s'==>'%s'", i, ptr->key, ptr->value);
    ptr = ptr->p_next;
    i++;
  }
  ESP_LOGD(TAG, "===============");
}

void lulz_global_set(char* key, char* value) {
  // Quit early if they gave us bad data
  if (key == NULL || value == NULL) {
    return;
  }

  // Current pointer in the linked list
  lulz_global_t* ptr = p_globals_first;
  // Pointer to last node we looked at
  lulz_global_t* last = NULL;

  // Walk the linked list looking for the global key
  // If we get out of here with ptr == NULL then key not found
  // If ptr is not-NULL then key is found and value must be replaced
  while (ptr != NULL) {
    if (strcmp(key, ptr->key) == 0) {
      break;
    }

    // Move to next and remember last
    last = ptr;
    ptr = ptr->p_next;
  }

  // Add a new node to the list
  if (ptr == NULL) {
    ESP_LOGD(TAG, "Saving '%s'='%s'", key, value);

    // Create some memory, globals exist until the device is rebooted so
    // don't bother about freeing this later
    ptr = (lulz_global_t*)util_heap_alloc_ext(sizeof(lulz_global_t));
    strncpy(ptr->key, key, LULZ_GLOBAL_MAX_SIZE);
    ptr->p_next = NULL;
    ptr->key[LULZ_GLOBAL_MAX_SIZE] = '\0';  // null terminate just in case
    if (p_globals_first == NULL) {
      p_globals_first = ptr;
    }
    if (last != NULL) {
      last->p_next = ptr;
    }
  }

  // By this point we have either an existing node or new node
  strncpy(ptr->value, value, LULZ_GLOBAL_MAX_SIZE);
  ptr->value[LULZ_GLOBAL_MAX_SIZE] = '\0';  // null terminate just in case

  lulz_global_print();
}

void task_lulzcode(void* pvParameters) {
  char* filename = (char*)pvParameters;
  lolcode_execute(filename, false);
  vTaskDelete(NULL);
}
