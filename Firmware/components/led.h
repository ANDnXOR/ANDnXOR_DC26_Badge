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
#ifndef COMPONENTS_LED_H_
#define COMPONENTS_LED_H_

#define LED_COUNT 31

#define LED_PATTERN_ROLLER_COASTER_COUNT 5
#define LED_PATTERN_BALLS_COUNT 5
typedef struct {
  float gravity;
  color_rgb_t colors[LED_PATTERN_BALLS_COUNT];
  int32_t start_height;
  float height[LED_PATTERN_BALLS_COUNT];
  float impact_velocity_start;
  float impact_velocity[LED_PATTERN_BALLS_COUNT];
  float time_since_last_bounce[LED_PATTERN_BALLS_COUNT];
  int32_t position[LED_PATTERN_BALLS_COUNT];
  uint32_t clock_since_last_bounce[LED_PATTERN_BALLS_COUNT];
  float dampening[LED_PATTERN_BALLS_COUNT];
} led_pattern_balls_t;

typedef struct {
  int8_t direction_red;
  int8_t direction_green;
  int8_t direction_blue;
  int8_t direction_yellow;
  int8_t index_red;
  int8_t index_green;
  int8_t index_blue;
  int8_t index_yellow;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t yellow;
  color_rgb_t rgb[LED_COUNT];
} led_triple_sweep_state_t;

extern uint8_t led_brightness_get();
extern void led_brightness_set(uint8_t brightness);
extern void led_clear();

/**
 * Set the delay between blinks of the eye LED
 *
 * @param delay	Delay between blinks
 */
extern void led_eye_blink_delay(uint16_t delay);

/**
 * Start and stop the blinking task
 */
extern void led_eye_blink_start();
extern void led_eye_blink_stop();

/**
 * Initialize the eye LED
 */
extern void led_eye_init();

/**
 * Set brightness (PWM) of the eye LED
 *
 * @param brightness	Brightness (0 = off, 255 = max)
 */
extern void led_eye_pwm(uint8_t brightness);

/**
 * Set the eye LED GPIO state, high or low
 *
 * @param high	True=on, False=off
 */
extern void led_eye_set(bool high);

/**
 * Start pulsing the eye LED
 */
extern void led_eye_pulse_start();

/**
 * Stop pulsing the eye LED
 */
extern void led_eye_pulse_stop();

extern void led_init();
extern void led_pattern_double_sweep(uint8_t* p_index,
                                     float* p_hue,
                                     float* p_value);

/**
 * @brief Initilize balls
 * @param p_balls : Pointer to balls
 */
extern void led_pattern_balls_init(led_pattern_balls_t* p_balls);

/**
 * @brief Bouncing ball LED pattern mode. Ported from tweaking4all
 * @param count : Number of balls
 * @param colors : RGB values for each ball
 */
extern void led_pattern_balls(led_pattern_balls_t* p_balls);

/**
 * @brief Flame LED mode
 */
extern void led_pattern_flame();
extern void led_pattern_hue(float* p_hue);

/**
 * @brief Do a kitt (knight rider) like pattern because of course
 * @param p_index : Pointer to current index
 * @param p_direction : Pointer to current direction
 */
extern void led_pattern_kitt(int8_t* p_index, int8_t* p_direction);

extern void led_pattern_polar();

/**
 * @brief Rainbow pattern because we can
 * @param p_hue : Pointer to hue for first LED
 * @param repeat : How many times to repeat the rainbow
 */
extern void led_pattern_rainbow(float* p_hue, uint8_t repeat);

/**
 * @brief Run LEDs in a roller coaster pattern (ala Win10)
 * @param indices : Positions of each LED ball in the pattern
 */
extern void led_pattern_roller_coaster(uint8_t positions[], color_rgb_t color);

/**
 * @brief Do a running lights animation of a specific color
 * @param red : Max Red value
 * @param green : Max Green value
 * @param blue : Max blue value
 */
extern void led_pattern_running_lights(uint8_t red,
                                       uint8_t green,
                                       uint8_t blue,
                                       uint8_t* p_position);

extern void led_pattern_sparkle(uint8_t* p_index);

/**
 * @brief LED pattern that has 3 sweeps mode for RED GREEN and BLUE
 * @param triple sweep : Struct holding triple sweep state
 */
extern void led_pattern_triple_sweep(led_triple_sweep_state_t *p_triple_sweep);

extern void led_set(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
extern void led_set_rgb(uint32_t index, color_rgb_t rgb);
extern void led_set_all(uint8_t r, uint8_t g, uint8_t b);
extern void led_set_all_rgb(color_rgb_t);

/**
 * Set an LED manually
 *
 * @param index		Index of the RGB LED to set
 * @param r			Red value to set
 * @param g			Green value to set
 * @param b			Blue value to set
 */
extern void led_set_raw(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
extern void led_show();
extern void led_test();

#endif /* COMPONENTS_LED_H_ */
