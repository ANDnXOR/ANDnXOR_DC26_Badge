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

#define SKI_ACCELERATION 0.015
#define SKI_BG COLOR_LIGHTBROWN
#define SKI_FG COLOR_BLUE
#define SKI_STEP_MS 100
#define SKI_X 102
#define SKI_Y 4
#define SKI_W 24
#define SKI_H 24
#define SKI_MARGIN 4
#define SKI_SPRITE_COUNT 32
#define SKI_WORLD_WIDTH 4
#define SKI_WORLD_HEIGHT 2
#define SKI_SPRITE_W 24
#define SKI_SPRITE_H 24
#define SKI_SPRITE_TYPE_COUNT 3
#define SKI_MBP_W 26
#define SKI_MBP_H 30
#define SKI_MBP_SPEED 3
#define SKI_TILT_ANGLE 30 /* threshold for changing right/left */

__attribute__((unused)) const static char* TAG = "MRMEESEEKS::Ski";

typedef struct {
  float x, y;
  int16_t dx, dy;
  uint8_t sprite_index;
} sprite_t;

typedef struct {
  sprite_t sprites[SKI_SPRITE_COUNT];
  int8_t angle;
  float distance;
  float velocity;
  float velocity_angled;
  color_565_t left_raw[SKI_W * SKI_H];
  color_565_t right_raw[SKI_W * SKI_H];
  color_565_t down_raw[SKI_W * SKI_H];
  color_565_t sprite_raw[SKI_SPRITE_TYPE_COUNT][SKI_SPRITE_W * SKI_SPRITE_H];
  volatile bool exit_flag;
} ski_state_t;

/**
 * Draw ski free game
 */
static void __draw(ski_state_t* p_state) {
  // Clear the screen
  gfx_fill_screen(SKI_BG);
  gfx_color_set(SKI_FG);

  // Draw the sprites
  for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
    sprite_t s = p_state->sprites[i];
    gfx_fill_rect(s.x - s.dx - 1, s.y - s.dy - 1, SKI_SPRITE_W + 2,
                  SKI_SPRITE_H + 2, SKI_BG);
    gfx_draw_raw(s.x, s.y, SKI_SPRITE_W, SKI_SPRITE_H,
                 p_state->sprite_raw[s.sprite_index]);
  }

  // Draw the skier
  switch (p_state->angle) {
    case -45:
      gfx_draw_raw(SKI_X, SKI_Y, SKI_W, SKI_H, p_state->left_raw);
      break;
    case 0:
      gfx_draw_raw(SKI_X, SKI_Y, SKI_W, SKI_H, p_state->down_raw);
      break;
    case 45:
      gfx_draw_raw(SKI_X, SKI_Y, SKI_W, SKI_H, p_state->right_raw);
      break;
  }

  char dist[32];
  sprintf(dist, "%dm", (int)p_state->distance);
  gfx_cursor_set((cursor_coord_t){0, 0});

  gfx_fill_rect(0, 0, 30, gfx_font_height(), SKI_BG);
  gfx_print(dist);
  gfx_push_screen_buffer();
}

static void __mbp() {
  color_565_t mbp_raw[SKI_MBP_W * SKI_MBP_H];
  gfx_load_raw(mbp_raw, "/sdcard/ski/mbp.raw", SKI_MBP_W * SKI_MBP_H);
  for (int16_t x = 0 - SKI_MBP_W; x < SKI_X - SKI_MBP_W; x += SKI_MBP_SPEED) {
    gfx_fill_rect(x - SKI_MBP_SPEED, SKI_Y + 3, SKI_MBP_SPEED, SKI_MBP_H,
                  SKI_BG);
    gfx_draw_raw(x, SKI_Y + 3, SKI_MBP_W, SKI_MBP_H, mbp_raw);
    gfx_push_screen_buffer();
    DELAY(50);
  }
  DELAY(1000);
}

static void __ski_task(void* data) {
  ski_state_t* p_state = (ski_state_t*)data;
  TickType_t ticks = 100 / portTICK_PERIOD_MS;

  while (!p_state->exit_flag) {
    TickType_t start = xTaskGetTickCount();

    // Move the sprites (not the skier)
    float dx = 0, dy = 0;

    // Determine the velocities
    switch (p_state->angle) {
      case -45:
        dx = p_state->velocity_angled;
        dy = 0 - p_state->velocity_angled;
        break;
      case 0:
        dx = 0;
        dy = 0 - p_state->velocity;
        break;
      case 45:
        dx = 0 - p_state->velocity_angled;
        dy = 0 - p_state->velocity_angled;
        break;
    }

    // Move the sprites
    for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
      p_state->sprites[i].x += dx;
      p_state->sprites[i].y += dy;
      p_state->sprites[i].dx = dx;
      p_state->sprites[i].dy = dy;

      // sprite is off the screen re-generate
      if (p_state->sprites[i].y < 0 - SKI_H) {
        bool overlap = true;

        // Keep trying utnil we know something was picked that does not overlap
        while (overlap) {
          // Move the sprite around
          p_state->sprites[i].x =
              (int16_t)util_random(0, LCD_WIDTH * SKI_WORLD_WIDTH) -
              ((SKI_WORLD_WIDTH / 2) *
               LCD_WIDTH);  // Generate sprites three screens wide
          p_state->sprites[i].y =
              util_random(0, LCD_HEIGHT * SKI_WORLD_HEIGHT) + LCD_HEIGHT;
          p_state->sprites[i].sprite_index =
              util_random(0, SKI_SPRITE_TYPE_COUNT);
          float x_min = p_state->sprites[i].x;
          float x_max = x_min + SKI_SPRITE_W;
          float y_min = p_state->sprites[i].y;
          float y_max = y_min + SKI_SPRITE_H;

          // Check all sprites before it for overlap, assume none until we
          // figure out otherwise
          overlap = false;
          for (uint8_t j = 0; j < SKI_SPRITE_COUNT; j++) {
            // Make sure we're not looking at ourselve
            if (i != j) {
              if (!(x_min > p_state->sprites[j].x + SKI_SPRITE_W) &&
                  !(x_max < p_state->sprites[j].x) &&
                  !(y_min > p_state->sprites[j].y + SKI_SPRITE_H) &&
                  !(y_max < p_state->sprites[j].y)) {
                overlap = true;
                break;
              }
            }
          }
        }
      }

      // Detect collisions
      if (p_state->sprites[i].x + SKI_W > SKI_X + SKI_MARGIN &&
          p_state->sprites[i].x < SKI_X + SKI_W - SKI_MARGIN &&
          p_state->sprites[i].y + SKI_H > SKI_Y + SKI_MARGIN &&
          p_state->sprites[i].y < SKI_Y + SKI_H - SKI_MARGIN) {
        p_state->exit_flag = true;
      }
    }

    p_state->distance += p_state->velocity;
    p_state->velocity += SKI_ACCELERATION;
    p_state->velocity_angled = sqrt(pow(p_state->velocity, 2));

    // Get controls from the user, first calculate an angle based on
    // accelerometer this may be overriden later by button input
    if (post_state_get()->accelerometer_ack > 0) {
      int16_t angle = drv_lis2de12_tilt_get();
      if (angle < 180 - SKI_TILT_ANGLE) {
        p_state->angle = -45;
      } else if (angle > 180 + SKI_TILT_ANGLE) {
        p_state->angle = 45;
      } else {
        p_state->angle = 0;
      }
    } else {
      // Ensure angle gets reset even if accelerometer is not working
      p_state->angle = 0;
    }

    // Read button input possibly overriding the accelerometer input
    if (btn_a()) {
      p_state->angle = -45;
    } else if (btn_b()) {
      p_state->angle = 45;
    }

    if (btn_left()) {
      btn_clear();

      char* label = "Paused";
      int16_t x, y;
      uint16_t w, h;
      gfx_font_set(font_large);
      gfx_text_bounds(label, 0, 0, &x, &y, &w, &h);
      cursor_coord_t cursor = {(LCD_WIDTH - w) / 2, (LCD_HEIGHT / 2) - h};

      // Print drop shadow
      gfx_cursor_set(cursor);
      gfx_color_set(COLOR_BLACK);
      gfx_print(label);

      // Print actual text
      cursor.x -= 2;
      cursor.y -= 2;
      gfx_cursor_set(cursor);
      gfx_color_set(COLOR_WHITE);
      gfx_print(label);
      gfx_ui_draw_left_button("Quit");
      gfx_ui_draw_right_button("Resume");
      gfx_push_screen_buffer();

      DELAY(200);
      btn_wait();

      if (btn_a()) {
        p_state->exit_flag = true;
      }

      // Cleanup
      gfx_fill_screen(COLOR_BLACK);
      btn_clear();
    }

    __draw(p_state);
    vTaskDelayUntil(&start, ticks);
  }

  vTaskDelete(NULL);
}

void ski() {
  char* sprite_files[] = {"/sdcard/ski/lift.raw", "/sdcard/ski/rock.raw",
                          "/sdcard/ski/tree.raw"};

  gfx_color_set(COLOR_GREEN);
  gfx_fill_screen(COLOR_BLACK);
  gfx_font_set(font_large);
  gfx_cursor_set((cursor_coord_t){0, 0});
  gfx_print("Ski Free\n");
  gfx_font_set(font_small);
  gfx_print(
      "Tilt badge side to side to steer.\n\nUse LEFT button to quit.\n\nAny "
      "button to continue...");
  gfx_push_screen_buffer();
  util_heap_stats_dump();
  btn_wait();
  btn_clear();

  // Initialize the skier
  ski_state_t* p_state = util_heap_alloc_ext(sizeof(ski_state_t));
  p_state->exit_flag = false;
  p_state->angle = 0;
  p_state->distance = 0;
  p_state->velocity = 2.0;
  p_state->velocity_angled = sqrt(pow(p_state->velocity, 2));
  gfx_load_raw(p_state->left_raw, "/sdcard/ski/skileft.raw", SKI_W * SKI_H);
  gfx_load_raw(p_state->right_raw, "/sdcard/ski/skiright.raw", SKI_W * SKI_H);
  gfx_load_raw(p_state->down_raw, "/sdcard/ski/skidown.raw", SKI_W * SKI_H);

  // Load the sprites
  for (uint8_t i = 0; i < SKI_SPRITE_TYPE_COUNT; i++) {
    gfx_load_raw(p_state->sprite_raw[i], sprite_files[i],
                 SKI_SPRITE_W * SKI_SPRITE_H);
  }

  // Initialize the sprites
  for (uint8_t i = 0; i < SKI_SPRITE_COUNT; i++) {
    bool overlap = true;
    while (overlap) {
      p_state->sprites[i].x =
          (int16_t)util_random(0, LCD_WIDTH * SKI_WORLD_WIDTH) -
          ((SKI_WORLD_WIDTH / 2) *
           LCD_WIDTH);  // Generate sprites three screens wide
      p_state->sprites[i].y =
          util_random(0, LCD_HEIGHT * SKI_WORLD_HEIGHT) + LCD_HEIGHT;
      p_state->sprites[i].sprite_index = util_random(0, SKI_SPRITE_TYPE_COUNT);
      float x_min = p_state->sprites[i].x;
      float x_max = x_min + SKI_SPRITE_W;
      float y_min = p_state->sprites[i].y;
      float y_max = y_min + SKI_SPRITE_H;

      // Check all sprites before it for overlap
      overlap = false;
      for (uint8_t j = 0; j < i; j++) {
        if (!(x_min > p_state->sprites[j].x + SKI_SPRITE_W) &&
            !(x_max < p_state->sprites[j].x) &&
            !(y_min > p_state->sprites[j].y + SKI_SPRITE_H) &&
            !(y_max < p_state->sprites[j].y)) {
          overlap = true;
          break;
        }
      }
    }
  }

  // Don't allow UI to interrupt us
  ui_allow_interrupt(false);

  // Startup ski free task
  static StaticTask_t task;
  TaskHandle_t handle = util_task_create(__ski_task, "Ski Free", 8192, p_state,
                                         TASK_PRIORITY_MEDIUM, &task);

  // Game failed :(
  if (handle == NULL) {
    return;
  }

  while (!p_state->exit_flag) {
    DELAY(100);
  }

  // Bring out Man Bear Pig
  __mbp();

  // Done, clean up
  ui_allow_interrupt(true);

  // Record high score
  if (p_state->distance > state_score_ski_get()) {
    state_score_ski_set(p_state->distance);
    // Set the unlock flag
    if (p_state->distance >= UNLOCK_BROADCAST_SKI_FREE_SCORE) {
      //First time for this unlock
      if ((state_unlock_get() & UNLOCK_SKI_FREE) == 0) {
        ui_popup_info("Ski Free Unlock Found!");
      }
      state_unlock_set(state_unlock_get() | UNLOCK_SKI_FREE);
      ui_menu_main_regenerate();
    }
  }
}
