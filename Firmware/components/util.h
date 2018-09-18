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
#ifndef COMPONENTS_UTIL_H_
#define COMPONENTS_UTIL_H_

#define LOG_MAROON "\x1B[0m"
#define LOG_RED "\x1B[31m"
#define LOG_GREEN "\x1B[32m"
#define LOG_YELLOW "\x1B[33m"
#define LOG_BLUE "\x1B[34m"
#define LOG_MAGENTA "\x1B[35m"
#define LOG_CYAN "\x1B[36m"
#define LOG_WHITE "\x1B[37m"

#define ROTATE_THRESHOLD 40
#define SHA256_DIGEST_SIZE_BYTES 65

#define DELAY(ms) vTaskDelay(ms / portTICK_PERIOD_MS)
#define MILLIS() (uint32_t)(xTaskGetTickCount() * portTICK_RATE_MS)
#define YIELD() vPortYield()

#ifndef SWAP_INT16_T
#define SWAP_INT16_T(a, b) \
  {                        \
    int16_t t = a;         \
    a = b;                 \
    b = t;                 \
  }
#endif

#ifndef UINT16_T_SWAP
#define UINT16_T_SWAP(num) ((num >> 8) | (num << 8))
#endif

//#ifndef MIN
//#define MIN(x,y) (x < y ? x : y)
//#endif
//
//#ifndef MAX
//#define MAX(x,y) (x > y ? x : y)
//#endif

typedef uint16_t color_565_t;
typedef uint32_t color_rgb_t;

/**
 * @brief Check bagel pin unlock
 */
extern void util_bagel_pin_unlock_check();

extern bool util_file_exists(const char* path);

/**
 * Computes SHA256 hash of a file. Results are stored in output buffer as base64
 * which is 64 bytes (+1 for null terminated)
 *
 * @param	path	Full path to file to hash
 * @param	output	Output buffer to store the hash
 */
extern void util_file_sha256(char* path, char* output);
extern uint32_t util_file_size(const char* path);

/**
 * @brief Dump heap stats to DEBUG
 */
extern void util_heap_stats_dump();

/**
 * @brief Allocate on external memory. Simple helper function
 * @param size : Size of heap block to allocate
 */
extern void* util_heap_alloc_ext(size_t size);

/**
 * @brief Re-allocate space on external memory. Simple helper function
 * @param ptr : Pointer to reallocate memory of
 * @param size: New size of heap block
 */
extern void* util_heap_realloc_ext(void* ptr, size_t size);

extern uint32_t util_hsv_to_rgb(float H, float S, float V);

/**
 * @brief Map a value between in min/max to out min/max
 * @param x : value to map
 * @param in_min : input value minimum
 * @param in_max : input value maximum
 * @param out_min : output value minimum
 * @param out_max : output value maximum
 */
extern int32_t util_map(int32_t x,
                        int32_t in_min,
                        int32_t in_max,
                        int32_t out_min,
                        int32_t out_max);

extern int util_mkdir_p(const char* path);
extern void util_nvs_init();

extern uint32_t util_random(uint32_t min, uint32_t max);

extern color_rgb_t util_565_to_rgb(color_565_t color);

/**
 * @brief Print text to console without hogging the CPU
 * @param text : Text to print
 */
extern void util_print_nice(char *text);

/**
 * Convert Red Green Blue values to a 565 color for LCD display. Most
 * significant bits are used in the 565 color.
 *
 * @param	r		Red value
 * @param 	b		Blue value
 * @param 	g		Green value
 *
 * @return 	565 value using the most significant bits of the R G B values.
 */
extern color_565_t util_rgb_to_565_discreet(uint8_t r, uint8_t g, uint8_t b);

/**
 * Convert an RGB value stored in a 32-bit int to 565 color suitable for LCD
 * display.
 *
 * @param	rgb		RGB color packed in a 32-bit int
 *
 * @return	565 value
 */
extern color_565_t util_rgb_to_565(color_rgb_t rgb);

/**
 * @brief Get 32 bits of the factory-unique MAC address as a semi-reliable
 * serial number
 * @return 32 bit serial
 */
extern uint32_t util_serial_get();

/**
 * @brief Take a screenshot and save it to sdcard
 */
extern void util_screenshot();

/**
 * @brief XORs a string with a key
 * @param input : Input buffer to XOR it may contain erroneous \0
 * @param output : Output buffer to write to, may contain erroneous \0
 * @param length : Actual length of buffer
 * @param key : Key to XOR with
 */
extern void util_string_xor(const char* input, char* output, size_t length, char* key);

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
extern TaskHandle_t util_task_create(TaskFunction_t function,
                                     const char* const name,
                                     uint32_t stack_depth,
                                     void* parameters,
                                     UBaseType_t priority,
                                     StaticTask_t* p_static_task);

/**
 * @brief Dump task list to terminal
 */
extern void util_task_stat_dump();

extern void util_update_global_brightness();

/**
 * @brief Validate an unlock code. State is updated
 * @param code : Code to validate
 * @return true if successful
 */
extern bool util_validate_unlock(const char* code);
#endif /* COMPONENTS_UTIL_H_ */
