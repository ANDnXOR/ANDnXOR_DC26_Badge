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
#define SHA_CHUNK_SIZE 512 /* Can be up to 8192 but lets save some stack */
#define SHA256_OUTPUT_SIZE_BYTES 32

// XORed unlock magic values
// Run xor.py in /mrmeeseeks to generate the code
// Example python xor.py "string"
static const char unlock_ae_goon_str[] = {0x23, 0x4b, 0x5d, 0x33,
                                          0x3d, 0x57, 0x4,  0x2};
static const size_t unlock_ae_goon_str_length = 8;

static const char unlock_pirate_str[] = {0x0,  0x28, 0x61, 0x11,
                                         0x2a, 0x7f, 0x3a, 0x24};
static const size_t unlock_pirate_str_length = 8;

static const char unlock_silk_str[] = {0xd,  0x4b, 0x9,  0x15,
                                       0x22, 0x65, 0x22, 0x2c};
static const size_t unlock_silk_str_length = 8;

static const char unlock_voicemail_str[] = {0x3,  0x4c, 0x76, 0x19,
                                            0x34, 0x0,  0x22};
static const size_t unlock_voicemail_str_length = 7;

static const char unlock_lanyard_owl_str[] = {0x1d, 0x30, 0x7b,
                                              0x11, 0x3c, 0x61};
static const size_t unlock_lanyard_owl_str_length = 6;

const static char* TAG = "UTIL";

/**
 * @brief Check bagel pin unlock
 */
void util_bagel_pin_unlock_check() {
  uint16_t unlock = state_unlock_get();
  bool bagel = (drv_greenpak_read_bagel_pin() == 1) &&
               (drv_greenpak_read_toast_pin() == 1);

  if ((unlock & UNLOCK_BAGEL) == 0) {
    if (bagel) {
      state_unlock_set(unlock | UNLOCK_BAGEL);
      state_save_indicate();
      ui_popup_info("You found bagel bling!");
      ESP_LOGD(TAG, "Bagel State: 0x%02x Unlock state: 0x%02x", bagel,
               state_unlock_get());
    }
  }
}

bool util_file_exists(const char* path) {
  struct stat s;
  return (stat(path, &s) == 0);
}

void util_file_sha256(char* path, char* hex_digest) {
  uint8_t hash[SHA256_OUTPUT_SIZE_BYTES];
  uint8_t buffer[SHA_CHUNK_SIZE];
  size_t bytes_read = 0;
  int32_t bytes_left = util_file_size(path);
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);

  FILE* file = fopen(path, "r");
  if (file) {
    while (bytes_left > 0) {
      bytes_read = fread(buffer, 1, SHA_CHUNK_SIZE, file);
      mbedtls_sha256_update(&ctx, buffer, bytes_read);
      bytes_left -= bytes_read;
    }

    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);
  }
  fclose(file);

  // Convert to hex digest string
  memset(hex_digest, 0, SHA256_DIGEST_SIZE_BYTES + 1);
  for (uint8_t i = 0; i < SHA256_OUTPUT_SIZE_BYTES; i++) {
    sprintf(hex_digest + (i * 2), "%02x", hash[i]);
  }
}

uint32_t util_file_size(const char* path) {
  struct stat st;
  stat(path, &st);
  return st.st_size;
}

/**
 * @brief Dump heap stats to DEBUG
 */
void util_heap_stats_dump() {
  ESP_LOGD(TAG,
           "%sHEAP Free: %d "
           "%sLowest: %d "
           "%sDMA: %d "
           "%sLargest Block: %d",
           LOG_CYAN, xPortGetFreeHeapSize(), LOG_YELLOW,
           xPortGetMinimumEverFreeHeapSize(), LOG_RED,
           heap_caps_get_free_size(MALLOC_CAP_DMA), LOG_BLUE, heap_caps_get_largest_free_block(MALLOC_CAP_DEFAULT));
}

/**
 * @brief Allocate on external memory. Simple helper function
 * @param size : Size of heap block to allocate
 */
inline void* util_heap_alloc_ext(size_t size) {
  return heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
}

/**
 * @brief Re-allocate space on external memory. Simple helper function
 * @param ptr : Pointer to reallocate memory of
 * @param size: New size of heap block
 */
inline void* util_heap_realloc_ext(void* ptr, size_t size) {
  return heap_caps_realloc(ptr, size, MALLOC_CAP_SPIRAM);
}

color_rgb_t util_hsv_to_rgb(float H, float S, float V) {
  H = fmodf(H, 1.0);

  float h = H * 6;
  uint8_t i = floor(h);
  float a = V * (1 - S);
  float b = V * (1 - S * (h - i));
  float c = V * (1 - (S * (1 - (h - i))));
  float rf, gf, bf;

  switch (i) {
    case 0:
      rf = V * 255;
      gf = c * 255;
      bf = a * 255;
      break;
    case 1:
      rf = b * 255;
      gf = V * 255;
      bf = a * 255;
      break;
    case 2:
      rf = a * 255;
      gf = V * 255;
      bf = c * 255;
      break;
    case 3:
      rf = a * 255;
      gf = b * 255;
      bf = V * 255;
      break;
    case 4:
      rf = c * 255;
      gf = a * 255;
      bf = V * 255;
      break;
    case 5:
    default:
      rf = V * 255;
      gf = a * 255;
      bf = b * 255;
      break;
  }

  uint8_t R = rf;
  uint8_t G = gf;
  uint8_t B = bf;

  color_rgb_t RGB = (R << 16) + (G << 8) + B;
  return RGB;
}

/**
 * @brief Map a value between in min/max to out min/max
 * @param x : value to map
 * @param in_min : input value minimum
 * @param in_max : input value maximum
 * @param out_min : output value minimum
 * @param out_max : output value maximum
 */
int32_t util_map(int32_t x,
                 int32_t in_min,
                 int32_t in_max,
                 int32_t out_min,
                 int32_t out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int util_mkdir_p(const char* path) {
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  const size_t len = strlen(path);
  char _path[PATH_MAX];
  char* p;

  errno = 0;

  /* Copy string so its mutable */
  if (len > sizeof(_path) - 1) {
    errno = ENAMETOOLONG;
    return -1;
  }
  strcpy(_path, path);

  /* Iterate the string */
  for (p = _path + 1; *p; p++) {
    if (*p == '/') {
      /* Temporarily truncate */
      *p = '\0';

      if (mkdir(_path, S_IRWXU) != 0) {
        if (errno != EEXIST)
          return -1;
      }

      *p = '/';
    }
  }

  if (mkdir(_path, S_IRWXU) != 0) {
    if (errno != EEXIST)
      return -1;
  }

  return 0;
}

void util_nvs_init() {
  esp_err_t ret;
  ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
    ESP_LOGE(TAG, "No free pages in NVS, erasing...");
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

/**
 * @brief Print text to console without hogging the CPU
 * @param text : Text to print
 */
void util_print_nice(char *text) {
  while (strlen(text) > 0) {
    uint32_t len = MIN(strlen(text), 32);
    printf("%.32s", text);
    text += len;
    YIELD();
  }
}

/**
 * Return a random number between min and max-1.
 */
uint32_t util_random(uint32_t min, uint32_t max) {
  if (max == min) {
    return min;
  }
  return (esp_random() % (max - min)) + min;
}

/**
 * Converts a 16-bit 565 color to RGB color suitable for LEDs
 */
color_rgb_t util_565_to_rgb(color_565_t color) {
  return ((0xF800 & color) << 8) | ((0x07E0 & color) << 5) |
         (0x1F & color) << 3;
}

color_565_t util_rgb_to_565_discreet(uint8_t red, uint8_t green, uint8_t blue) {
  uint16_t b = (blue >> 3) & 0x1f;
  uint16_t g = ((green >> 2) & 0x3f) << 5;
  uint16_t r = ((red >> 3) & 0x1f) << 11;

  uint16_t c = (uint16_t)(r | g | b);
  return c;
}

color_565_t util_rgb_to_565(color_rgb_t rgb) {
  uint8_t r = (rgb >> 16) & 0xFF;
  uint8_t g = (rgb >> 8) & 0xFF;
  uint8_t b = rgb & 0xFF;
  return util_rgb_to_565_discreet(r, g, b);
}

/**
 * @brief Get 32 bits of the factory-unique MAC address as a semi-reliable
 * serial number
 * @return 32 bit serial
 */
uint32_t util_serial_get() {
  uint8_t serial[6];
  esp_efuse_mac_get_default(serial);
  return serial[2] << 24 | serial[3] << 16 | serial[4] << 8 | serial[5];
}

/**
 * @brief Wrapper to create tasks in a more heap friendly way
 * @param function : Function to run for the task
 * @param name : Name of the task to use throughout the badge
 * @param stack_depth : How much stack to allocate
 * @param parameters : User data to pass to task
 * @param priority : Priority to assign to the task
 * @param p_static_task : Static task reference we should return to
 * @return Newly created task handle
 */
TaskHandle_t util_task_create(TaskFunction_t function,
                              const char* const name,
                              uint32_t stack_depth,
                              void* parameters,
                              UBaseType_t priority,
                              StaticTask_t* p_static_task) {
  util_heap_stats_dump();

  StackType_t* p_stack =
      (StackType_t*)util_heap_alloc_ext(stack_depth * sizeof(StackType_t));
  ESP_LOGD(TAG, "Creating static task '%s' with stack size %d bytes", name,
           stack_depth);
  util_heap_stats_dump();
  return xTaskCreateStaticPinnedToCore(function, name, stack_depth, parameters,
                                       priority, p_stack, p_static_task,
                                       APP_CPU_NUM);
  //	TaskHandle_t handle;
  //	xTaskCreate(function, name, stack_depth, parameters, priority, &handle);
  //	return handle;
}

/**
 * @brief Take a screenshot and save it to sdcard
 */
void util_screenshot() {
  // Grab the current screen
  color_565_t* buffer = (color_565_t*)util_heap_alloc_ext(LCD_BUFFER_SIZE);
  gfx_screen_buffer_copy(buffer);

  // Create a path to the screenshot
  char path[256];
  sprintf(path, "/sdcard/screenshot-%d.bmp", time_manager_now_sec());

  // Generate the bitmap in memory
  BMP* bmp = BMP_Create(LCD_WIDTH, LCD_HEIGHT, 24);
  for (uint16_t x = 0; x < LCD_WIDTH; x++) {
    for (uint16_t y = 0; y < LCD_HEIGHT; y++) {
      color_565_t value = UINT16_T_SWAP(buffer[(y * LCD_WIDTH) + x]);
      color_rgb_t rgb = util_565_to_rgb(value);
      uint8_t r = (rgb >> 16);
      uint8_t g = (rgb >> 8);
      uint8_t b = rgb;
      BMP_SetPixelRGB(bmp, x, y, r, g, b);
    }
  }

  // Write
  BMP_WriteFile(bmp, path);

  // Cleanup
  BMP_Free(bmp);
  free(buffer);

  ESP_LOGD(TAG, "Screenshot saved to '%s'", path);
}

/**
 * @brief XORs a string with a key
 * @param input : Input buffer to XOR it may contain erroneous \0
 * @param output : Output buffer to write to, may contain erroneous \0
 * @param length : Actual length of buffer
 * @param key : Key to XOR with
 */
void util_string_xor(const char* input,
                     char* output,
                     size_t length,
                     char* key) {
  size_t key_len = strlen(key);
  for (size_t i = 0; i < length; i++) {
    *output++ = *input++ ^ key[i % key_len];
  }
  *output = '\0';
}

/**
 * @brief Dump task list to terminal
 */
void util_task_stat_dump() {
  char* buffer = util_heap_alloc_ext(60000);
  vTaskGetRunTimeStats(buffer);
  printf("%s\n\n", buffer);
  free(buffer);
}

void util_update_global_brightness() {
  uint8_t brightness = state_brightness_get();

  // Ensure we have at least default brightness value if brightness is invalid
  if (brightness < STATE_BRIGHTNESS_MIN || brightness > STATE_BRIGHTNESS_MAX) {
    ESP_LOGE(TAG, "Global brightness value invalid. Using Default");
    brightness = STATE_BRIGHTNESS_DEFAULT;
  }

  ESP_LOGD(TAG, "Setting brightness to %d", brightness);

  // Now adjust brightness value to a 0 to 255
  brightness *= 255 / STATE_BRIGHTNESS_MAX;

  // Backlight is always set to GCC
  drv_ili9225_backlight_set(255);

  // Set GCC value
  drv_is31fl_gcc_set(brightness);
}

/**
 * @brief Validate an unlock code. State is updated
 * @param code : Code to validate
 * @return true if successful
 */
bool util_validate_unlock(const char* code) {
  bool result = false;

  // XOR silk screen string
  char ae_goon[unlock_ae_goon_str_length + 1];
  util_string_xor(unlock_ae_goon_str, ae_goon, unlock_ae_goon_str_length,
                  STRING_ENCRYPTION_KEY);

  char silk[unlock_silk_str_length + 1];
  util_string_xor(unlock_silk_str, silk, unlock_silk_str_length,
                  STRING_ENCRYPTION_KEY);

  char pirate[unlock_pirate_str_length + 1];
  util_string_xor(unlock_pirate_str, pirate, unlock_pirate_str_length,
                  STRING_ENCRYPTION_KEY);

  char voicemail[unlock_voicemail_str_length + 1];
  util_string_xor(unlock_voicemail_str, voicemail, unlock_voicemail_str_length,
                  STRING_ENCRYPTION_KEY);

  char lanyard_owl[unlock_lanyard_owl_str_length + 1];
  util_string_xor(unlock_lanyard_owl_str, lanyard_owl,
                  unlock_lanyard_owl_str_length, STRING_ENCRYPTION_KEY);

  uint16_t unlock_state = state_unlock_get();

  if (strlen(code) > 8) {
    return false;
  }

  // AE Goon unlock
  if (strcmp(code, ae_goon) == 0) {
    state_unlock_set(unlock_state | UNLOCK_AE_GOON);
    state_save_indicate();
    ui_popup_info("New A&E Goon Bling Available!");
    result = true;
  }

  // Silk Screen unlock
  else if (strcmp(code, silk) == 0) {
    if ((unlock_state & UNLOCK_SILK) == 0) {
      state_unlock_set(unlock_state | UNLOCK_SILK);
      botnet_level_up();
      state_save_indicate();
      ui_popup_info("Botnet level up!");
    }
    result = true;
  }

  // Whiskey Pirate Mode
  else if (strcmp(code, pirate) == 0) {
    state_unlock_set(unlock_state | UNLOCK_BANDANA);
    state_save_indicate();
    ui_popup_info("New botnet payload available!");
    result = true;
  }

  // Lanyard / Owl Unlock
  else if (strncasecmp(code, lanyard_owl, unlock_lanyard_owl_str_length) == 0) {
    state_unlock_set(unlock_state | UNLOCK_LANYARD_OWL);
    state_save_indicate();
    ESP_LOGD(TAG, "Lanyard Owl Bling Unlock");
    ui_popup_info("Owl Bling Unlock!");
    result = true;
  }

  // Voicemail / telephreak unlock
  else if (strncasecmp(code, voicemail, unlock_voicemail_str_length) == 0) {
    state_unlock_set(unlock_state | UNLOCK_VOICEMAIL);
    state_save_indicate();
    ESP_LOGD(TAG, "Voicemail unlock");
    ui_popup_info("Telephreak Bling Unlock!");
    result = true;
  }

  // Valid boost unlock
  else {
    // Verify in valid range
    for (uint8_t i = 0; i < 8; i++) {
      if (code[i] < 32 || code[i] > 126) {
        return false;
      }
    }

    // Construct the code in parallel and validate
    char gen_code[9];
    gen_code[0] = code[0];  // First passes through
    gen_code[1] = MAX(code[0] / 2, 32);
    gen_code[2] = (util_serial_get() % 95) + 32;
    gen_code[3] = code[3];  // Passes through

    if (code[3] > code[0]) {
      gen_code[4] = 'A';
    } else {
      gen_code[4] = 'B';
    }

    uint16_t sum =
        (uint16_t)gen_code[1] + (uint16_t)gen_code[2] + (uint16_t)gen_code[3];
    snprintf(gen_code + 5, 4, "%d", sum);
    gen_code[7] = code[7];  // Passes through

    ESP_LOGD(TAG, "code = '%s' gen_code = '%s'", code, gen_code);
    result = strcmp(code, gen_code) == 0;

    if (result) {
      state_unlock_set(unlock_state | UNLOCK_LULZCODE);
      ui_popup_info("New botnet payload available!");
    }
  }

  return result;
}

/*
 * reallocarray() is not available on ESP32 so we just use the OpenBSD
 * implementation
 *
 * This is sqrt(SIZE_MAX+1), as s1*s2 <= SIZE_MAX
 * if both s1 < MUL_NO_OVERFLOW and s2 < MUL_NO_OVERFLOW
 */
#define MUL_NO_OVERFLOW ((size_t)1 << (sizeof(size_t) * 4))

void* reallocarray(void* optr, size_t nmemb, size_t size) {
  if ((nmemb >= MUL_NO_OVERFLOW || size >= MUL_NO_OVERFLOW) && nmemb > 0 &&
      SIZE_MAX / nmemb < size) {
    errno = ENOMEM;
    return NULL;
  }
  return realloc(optr, size * nmemb);
}
