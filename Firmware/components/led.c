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

#ifndef CONFIG_BADGE_TYPE_STANDALONE
#define LED_EYE_DUTY 4000
#define LED_EYE_DUTY_MAX 5000

static ledc_channel_config_t ledc_channel = {.channel = LEDC_CHANNEL_0,
                                             .duty = 0,
                                             .gpio_num = LED_EYE_PIN,
                                             .speed_mode = LEDC_HIGH_SPEED_MODE,
                                             .timer_sel = LEDC_TIMER_0};
#endif

#define LED_COUNT_INTERNAL 32

static const uint8_t gamma_values[] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,
    1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   2,   2,   2,   2,
    2,   2,   2,   2,   3,   3,   3,   3,   3,   3,   3,   4,   4,   4,   4,
    4,   5,   5,   5,   5,   6,   6,   6,   6,   7,   7,   7,   7,   8,   8,
    8,   9,   9,   9,   10,  10,  10,  11,  11,  11,  12,  12,  13,  13,  13,
    14,  14,  15,  15,  16,  16,  17,  17,  18,  18,  19,  19,  20,  20,  21,
    21,  22,  22,  23,  24,  24,  25,  25,  26,  27,  27,  28,  29,  29,  30,
    31,  32,  32,  33,  34,  35,  35,  36,  37,  38,  39,  39,  40,  41,  42,
    43,  44,  45,  46,  47,  48,  49,  50,  50,  51,  52,  54,  55,  56,  57,
    58,  59,  60,  61,  62,  63,  64,  66,  67,  68,  69,  70,  72,  73,  74,
    75,  77,  78,  79,  81,  82,  83,  85,  86,  87,  89,  90,  92,  93,  95,
    96,  98,  99,  101, 102, 104, 105, 107, 109, 110, 112, 114, 115, 117, 119,
    120, 122, 124, 126, 127, 129, 131, 133, 135, 137, 138, 140, 142, 144, 146,
    148, 150, 152, 154, 156, 158, 160, 162, 164, 167, 169, 171, 173, 175, 177,
    180, 182, 184, 186, 189, 191, 193, 196, 198, 200, 203, 205, 208, 210, 213,
    215, 218, 220, 223, 225, 228, 231, 233, 236, 239, 241, 244, 247, 249, 252,
    255};

const static unsigned char led_address[LED_COUNT_INTERNAL][3] = {
    {0x00, 0x10, 0x20}, {0x02, 0x12, 0x22}, {0x04, 0x14, 0x24},
    {0x06, 0x16, 0x26}, {0x08, 0x18, 0x28}, {0x0A, 0x1A, 0x2A},
    {0x0C, 0x1C, 0x2C}, {0x0E, 0x1E, 0x2E}, {0x30, 0x40, 0x50},
    {0x32, 0x42, 0x52}, {0x34, 0x44, 0x54}, {0x36, 0x46, 0x56},
    {0x38, 0x48, 0x58}, {0x3A, 0x4A, 0x5A}, {0x3C, 0x4C, 0x5C},
    {0x3E, 0x4E, 0x5E}, {0x60, 0x70, 0x80}, {0x62, 0x72, 0x82},
    {0x64, 0x74, 0x84}, {0x66, 0x76, 0x86}, {0x68, 0x78, 0x88},
    {0x6A, 0x7A, 0x8A}, {0x6C, 0x7C, 0x8C}, {0x6E, 0x7E, 0x8E},
    {0x90, 0xA0, 0xB0}, {0x92, 0xA2, 0xB2}, {0x94, 0xA4, 0xB4},
    {0x96, 0xA6, 0xB6}, {0x98, 0xA8, 0xB8}, {0x9A, 0xAA, 0xBA},
    {0x9C, 0xAC, 0xBC}, {0x9E, 0xAE, 0xBE}
    /*{ 0x9E, 0xAE, 0xBE }*/
};

const static char* TAG = "MRMEESEEKS::LED";

unsigned char led_memory[ISSI_ADDR_MAX];
static volatile uint16_t m_led_eye_blink_delay = 1000;
static uint8_t m_brightness = 10;
static bool m_led_eye_blink_run = false;
static bool m_led_eye_pulse_run = false;

void __blink_eye_task(void* params) {
  m_led_eye_blink_run = true;
  while (m_led_eye_blink_run) {
    led_eye_set(true);
    DELAY(m_led_eye_blink_delay);
    led_eye_set(false);
    DELAY(m_led_eye_blink_delay);
  }

  vTaskDelete(NULL);
}

void __pulse_eye_task(void* params) {
  ESP_LOGD(TAG, "Starting eye LED task");
  m_led_eye_pulse_run = true;
  int16_t brightness = 0;
  int8_t delta = 1;

  while (m_led_eye_pulse_run) {
    led_eye_pwm(brightness);
    brightness += delta;
    DELAY(50);

    if (brightness >= 70) {
      delta = -1;
    } else if (brightness < 0) {
      delta = 1;
    }

    if (brightness < 0) {
      brightness = 0;
    }
  }
  vTaskDelete(NULL);
}

uint8_t led_brightness_get() {
  return m_brightness;
}

void led_brightness_set(uint8_t brightness) {
  m_brightness = brightness;
  drv_is31fl_gcc_set(m_brightness);
}

void led_clear() {
  led_set_all(0, 0, 0);
  led_show();
}

void led_eye_blink_delay(uint16_t delay) {
  m_led_eye_blink_delay = delay;
}

void led_eye_blink_start() {
  if (!m_led_eye_blink_run) {
    xTaskCreatePinnedToCore(&__blink_eye_task, "Eye LED Blink", 4096, NULL,
                            TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);
  }
}

void led_eye_blink_stop() {
  m_led_eye_blink_run = false;
}

void led_eye_pwm(uint8_t brightness) {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  float percent = (float)brightness / 255.0;
  uint16_t new_value = (uint16_t)(percent * LED_EYE_DUTY_MAX);
  ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, new_value);
  ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
#endif
}

void led_eye_set(bool high) {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  gpio_set_level(LED_EYE_PIN, high);
#endif
}

void led_eye_init() {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  gpio_set_direction(LED_EYE_PIN, GPIO_MODE_OUTPUT);
#endif
}

void led_eye_pulse_start() {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  xTaskCreatePinnedToCore(&__pulse_eye_task, "Eye LED Pulse", 4096, NULL,
                          TASK_PRIORITY_LOW, NULL, APP_CPU_NUM);
#endif
}

void led_eye_pulse_stop() {
  m_led_eye_pulse_run = false;
}

void led_init() {
  drv_is31fl_init();
  for (uint8_t i = 0; i < ISSI_ADDR_MAX; i++) {
    led_memory[i] = 0;
  }

#ifndef CONFIG_BADGE_TYPE_STANDALONE
  // Initialize the LEDC peripheral for the eye
  gpio_set_direction(LED_EYE_PIN, GPIO_MODE_OUTPUT);

  /*
   * Prepare and set configuration of timers
   * that will be used by LED Controller
   */
  ledc_timer_config_t ledc_timer = {
      .bit_num = LEDC_TIMER_13_BIT,        // resolution of PWM duty
      .freq_hz = LED_EYE_DUTY_MAX,         // frequency of PWM signal
      .speed_mode = LEDC_HIGH_SPEED_MODE,  // timer mode
      .timer_num = LEDC_TIMER_0            // timer index
  };
  // Set configuration of timer0 for high speed channels
  ledc_timer_config(&ledc_timer);

  /*
   * Prepare individual configuration
   * for each channel of LED Controller
   * by selecting:
   * - controller's channel number
   * - output duty cycle, set initially to 0
   * - GPIO number where LED is connected to
   * - speed mode, either high or low
   * - timer servicing selected channel
   *   Note: if different channels use one timer,
   *         then frequency and bit_num of these channels
   *         will be the same
   */

  ledc_channel_config(&ledc_channel);
  ledc_fade_func_install(0);
  ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, LED_EYE_DUTY);
  ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
#endif
}

inline void led_set_raw(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
  if (index < LED_COUNT_INTERNAL) {
    led_memory[led_address[index][0]] = b;
    led_memory[led_address[index][1]] = g;
    led_memory[led_address[index][2]] = r;
  }
}

void led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  if (index < LED_COUNT) {
    led_memory[led_address[index][0]] = gamma_values[b / 2];
    led_memory[led_address[index][1]] = gamma_values[g / 2];
    led_memory[led_address[index][2]] = gamma_values[r / 2];
  }
#endif
}

void led_set_rgb(uint32_t index, color_rgb_t rgb) {
  led_set(index, (rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

void led_set_all(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    led_set(i, r, g, b);
  }
}

void led_set_all_rgb(color_rgb_t rgb) {
  for (uint32_t i = 0; i < LED_COUNT; i++) {
    led_set_rgb(i, rgb);
  }
}

void led_show() {
#ifndef CONFIG_BADGE_TYPE_STANDALONE
  for (uint8_t i = 0; i < LED_COUNT_INTERNAL; i++) {
    for (uint8_t ii = 0; ii < 3; ii++) {
      uint8_t address = led_address[i][ii];
      if (i < LED_COUNT) {
        // Actual LEDS halve their brightness
        drv_is31fl_send_value(address, led_memory[address] / 2);
      } else {
        drv_is31fl_send_value(address, led_memory[address]);
      }
    }
  }
#endif
}

void led_test() {
  while (1) {
    ESP_LOGD(TAG, "Testing Red");
    for (uint8_t i = 0; i < LED_COUNT; i++) {
      led_set(i, 255, 0, 0);
      led_show();
      DELAY(100);
    }

    ESP_LOGD(TAG, "Testing Green");
    for (uint8_t i = 0; i < LED_COUNT; i++) {
      led_set(i, 0, 255, 0);
      led_show();
      DELAY(100);
    }

    ESP_LOGD(TAG, "Testing Blue");
    for (uint8_t i = 0; i < LED_COUNT; i++) {
      led_set(i, 0, 0, 255);
      led_show();
      DELAY(100);
    }

    // HSV cycle
    for (uint8_t i = 0; i < 3; i++) {
      for (float h = 0; h < 1; h += 0.01) {
        color_rgb_t rgb = util_hsv_to_rgb(h, 1.0, 1.0);
        led_set_all_rgb(rgb);
        led_show();
        DELAY(50);
      }
    }

    led_clear();
    DELAY(1000);
  }
}

/**
 * @brief Initilize balls
 * @param p_balls : Pointer to balls
 */
void led_pattern_balls_init(led_pattern_balls_t* p_balls) {
  p_balls->start_height = 1;
  p_balls->gravity = -9.81;
  p_balls->impact_velocity_start =
      sqrt(-2 * p_balls->gravity * p_balls->start_height);

  // Initialize the led balls state
  for (int i = 0; i < LED_PATTERN_BALLS_COUNT; i++) {
    p_balls->clock_since_last_bounce[i] = MILLIS();
    p_balls->height[i] = p_balls->start_height;
    p_balls->position[i] = 0;
    p_balls->impact_velocity[i] = p_balls->impact_velocity_start;
    p_balls->time_since_last_bounce[i] = 0;
    p_balls->dampening[i] = 0.90 - (float)i / pow(LED_PATTERN_BALLS_COUNT, 2);
    p_balls->colors[i] =
        util_hsv_to_rgb((float)util_random(0, 100) / 100.0, 1, 1);
  }
}

/**
 * @brief Bouncing ball LED pattern mode. Ported from tweaking4all
 * @param p_balls : Pointer to balls state
 */
void led_pattern_balls(led_pattern_balls_t* p_balls) {
  for (int i = 0; i < LED_PATTERN_BALLS_COUNT; i++) {
    p_balls->time_since_last_bounce[i] =
        MILLIS() - p_balls->clock_since_last_bounce[i];
    p_balls->height[i] =
        0.5 * p_balls->gravity *
            pow(p_balls->time_since_last_bounce[i] / 1000, 2.0) +
        p_balls->impact_velocity[i] * p_balls->time_since_last_bounce[i] / 1000;

    if (p_balls->height[i] < 0) {
      p_balls->height[i] = 0;
      p_balls->impact_velocity[i] =
          p_balls->dampening[i] * p_balls->impact_velocity[i];
      p_balls->clock_since_last_bounce[i] = MILLIS();

      // Bouncing has stopped, start over
      if (p_balls->impact_velocity[i] < 0.01) {
        p_balls->impact_velocity[i] = p_balls->impact_velocity_start;
        p_balls->colors[i] =
            util_hsv_to_rgb((float)util_random(0, 100) / 100.0, 1, 1);
      }
    }
    p_balls->position[i] =
        round(p_balls->height[i] * (LED_COUNT - 1) / p_balls->start_height);
  }

  for (int i = 0; i < LED_PATTERN_BALLS_COUNT; i++) {
    led_set_rgb(p_balls->position[i], p_balls->colors[i]);
  }
  led_show();
  led_set_all(0, 0, 0);
}

void led_pattern_double_sweep(uint8_t* p_index, float* p_hue, float* p_value) {
  color_rgb_t rgb = util_hsv_to_rgb(*p_hue, 1.0, *p_value);
  led_set_rgb(15, rgb);
  led_set_rgb(15 + *p_index, rgb);
  led_set_rgb(15 - *p_index, rgb);
  led_show();

  (*p_index)++;
  if (*p_index > 15) {
    *p_index = 0;

    // Ensure hue wraps around
    *p_hue += 0.1;
    if (*p_hue >= 1.0) {
      *p_hue -= 1.0;
    }

    // Alternate light / dark
    if (*p_value > 0) {
      *p_value = 0;
    } else {
      *p_value = 1.0;
    }
  }
}

/**
 * @brief LED pattern that has 3 sweeps mode for RED GREEN and BLUE
 * @param triple sweep : Struct holding triple sweep state
 */
void led_pattern_triple_sweep(led_triple_sweep_state_t* p_triple_sweep) {
  // Ensure indices are within range
  while (p_triple_sweep->index_red < 0) {
    p_triple_sweep->index_red += LED_COUNT;
  }
  while (p_triple_sweep->index_green < 0) {
    p_triple_sweep->index_green += LED_COUNT;
  }
  while (p_triple_sweep->index_blue < 0) {
    p_triple_sweep->index_blue += LED_COUNT;
  }
  while (p_triple_sweep->index_yellow < 0) {
    p_triple_sweep->index_yellow += LED_COUNT;
  }
  while (p_triple_sweep->index_red >= LED_COUNT) {
    p_triple_sweep->index_red -= LED_COUNT;
  }
  while (p_triple_sweep->index_green >= LED_COUNT) {
    p_triple_sweep->index_green -= LED_COUNT;
  }
  while (p_triple_sweep->index_blue >= LED_COUNT) {
    p_triple_sweep->index_blue -= LED_COUNT;
  }
  while (p_triple_sweep->index_yellow >= LED_COUNT) {
    p_triple_sweep->index_yellow -= LED_COUNT;
  }

  // Apply sweep colors
  p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_red] = p_triple_sweep->red
                                                            << 16;
  p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_green] =
      p_triple_sweep->green << 8;
  p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_blue] =
      p_triple_sweep->blue;
  p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_yellow] =
      (p_triple_sweep->yellow << 16) | (p_triple_sweep->yellow << 8);

  led_set_rgb((uint8_t)p_triple_sweep->index_red,
              p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_red]);
  led_set_rgb((uint8_t)p_triple_sweep->index_green,
              p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_green]);
  led_set_rgb((uint8_t)p_triple_sweep->index_blue,
              p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_blue]);
  led_set_rgb((uint8_t)p_triple_sweep->index_yellow,
              p_triple_sweep->rgb[(uint8_t)p_triple_sweep->index_yellow]);
  led_show();

  // Move indices, assume next run around range will be checked
  p_triple_sweep->index_red += p_triple_sweep->direction_red;
  p_triple_sweep->index_green += p_triple_sweep->direction_green;
  p_triple_sweep->index_blue += p_triple_sweep->direction_blue;
  p_triple_sweep->index_yellow += p_triple_sweep->direction_yellow;

  // Randomly change directions (sometimes) of one sweep color
  uint8_t r = util_random(0, 100);
  if (r == 0) {
    p_triple_sweep->direction_red *= -1;
    p_triple_sweep->red = util_random(180, 255);
  } else if (r == 1) {
    p_triple_sweep->direction_green *= -1;
    p_triple_sweep->green = util_random(180, 255);
  } else if (r == 2) {
    p_triple_sweep->direction_blue *= -1;
    p_triple_sweep->blue = util_random(180, 255);
  } else if (r == 3) {
    p_triple_sweep->direction_yellow *= -1;
    p_triple_sweep->yellow = util_random(180, 255);
  }
}

/**
 * @brief Flame LED mode
 */
void led_pattern_flame() {
  uint8_t red, green;
  for (int x = 0; x < LED_COUNT; x++) {
    // 1 in 4 chance of pixel changing
    if (util_random(0, 4) == 0) {
      // Pick a random red color and ensure green never exceeds it ensuring some
      // shade of red orange or yellow
      red = util_random(180, 255);
      green = util_random(0, red);
      led_set(x, red, green, 0);
    }
  }
  led_show();
}

void led_pattern_hue(float* p_hue) {
  float hue = *p_hue;

  led_set_all_rgb(util_hsv_to_rgb(hue, 1.0, 1.0));
  led_show();

  hue += 0.02;
  if (hue >= 1) {
    hue -= 1;
  }
  *p_hue = hue;
}

/**
 * @brief Do a kitt (knight rider) like pattern because of course
 * @param p_index : Pointer to current index
 * @param p_direction : Pointer to current direction
 */
void led_pattern_kitt(int8_t* p_index, int8_t* p_direction) {
  int8_t index;
  led_set_all(0, 0, 0);

  for (int8_t i = 0; i < 4; i++) {
    if (*p_direction > 0) {
      index = *p_index - i;
    } else {
      index = *p_index + i;
    }

    if (index >= 0 && index < LED_COUNT) {
      led_set(index, 255 / (i + 1), 0, 0);
    }
  }

  led_show();

  if (*p_direction > 0) {
    (*p_index) += 2;
    if (*p_index >= LED_COUNT) {
      *p_index = LED_COUNT - 1;
      *p_direction = -1;
    }
  } else {
    (*p_index) -= 2;
    if (*p_index < 0) {
      *p_index = 0;
      *p_direction = 1;
    }
  }
}

void led_pattern_polar() {
  accel_axis_t accel = drv_lis2de12_get();   // Get current accelerometer value
  uint16_t angle = drv_lis2de12_tilt_get();  // Get calculated tilt

  /**
   * Adjust brightness of LEDs based on Z axis
   * -64 resting flat on desk
   * 64 resting flat on desk upside down
   * 0 while worn
   */
  uint8_t brightness = 4 * (64 - MIN(abs(accel.z), 64));

  if (angle < 87 || angle > 273) {
    return;
  }

  // Normalize to 0
  angle -= 87;

  uint8_t index = angle / 6;
  led_set_all(0, 0, 0);
  for (uint8_t i = 0; i < 5; i++) {
    int8_t ii = (index - 2) + i;
    if (index < LED_COUNT) {
      led_set(ii, brightness, 0, 0);
    }
  }
  led_show();
}

/**
 * @brief Rainbow pattern because we can
 * @param p_hue : Pointer to hue for first LED
 * @param repeat : How many times to repeat the rainbow
 */
void led_pattern_rainbow(float* p_hue, uint8_t repeat) {
  float hue_step = 1.0 / ((float)LED_COUNT / (float)repeat);
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    color_rgb_t rgb = util_hsv_to_rgb(*p_hue + (hue_step * i), 1, 1);
    led_set_rgb(i, rgb);
  }
  led_show();

  *p_hue += 0.05;
  if (*p_hue > 1) {
    *p_hue -= 1;
  }
}

/**
 * @brief Run LEDs in a roller coaster pattern (ala Win10)
 * @param indices : Positions of each LED ball in the pattern
 */
void led_pattern_roller_coaster(uint8_t positions[], color_rgb_t color) {
  led_set_all(0, 0, 0);
  for (uint8_t i = 0; i < LED_PATTERN_ROLLER_COASTER_COUNT; i++) {
    float pos = (float)positions[i];
    float rad = (pos / (float)(LED_COUNT - 1)) * 2 * M_PI;
    float velocity =
        (-1.75) * cosf(rad) - 2.25;  // Ensure velocity is between -0.5 and -4.0
    pos += velocity;
    if (pos < 0) {
      pos += LED_COUNT;
    }
    positions[i] = (uint8_t)pos;

    led_set_rgb(positions[i], color);
  }

  led_show();
}

void led_pattern_sparkle(uint8_t* p_index) {
  uint8_t mode = util_random(0, 2);

  switch (mode) {
    case 0:
      *p_index = util_random(0, LED_COUNT);
      led_set(*p_index, 255, 255, 255);
      break;
    case 1:;
      float hue = (float)util_random(0, 100) / 100.0;
      led_set_rgb(*p_index, util_hsv_to_rgb(hue, 1.0, 1.0));
      break;
  }
  led_show();
}

/**
 * @brief Do a running lights animation of a specific color, ported from
 * tweaking4all
 * @param red : Max Red value
 * @param green : Max Green value
 * @param blue : Max blue value
 */
void led_pattern_running_lights(uint8_t red,
                                uint8_t green,
                                uint8_t blue,
                                uint8_t* p_position) {
  for (int i = 0; i < LED_COUNT; i++) {
    led_set(i, ((sin(i + *p_position) * 127 + 128) / 255) * red,
            ((sin(i + *p_position) * 127 + 128) / 255) * green,
            ((sin(i + *p_position) * 127 + 128) / 255) * blue);
  }

  led_show();

  (*p_position)++;
  if (*p_position >= LED_COUNT * 2) {
    *p_position = 0;
  }
}
