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

#include "adafruit/gfxfont.h"
#include "computerfont11.h"
#include "computerfont14.h"
#include "computerfont60.h"
#include "ibm4.h"
#include "inconsolata14pt7b.h"
#include "monaco10pt7b.h"
#include "monaco8pt7b.h"
#include "tandy8.h"

#include "gif_lib_private.h"  //YOLO

#define __FONT_X_LARGE Computerfont60pt7b
#define __FONT_LARGE Computerfont14pt7b
#define __FONT_MEDIUM Px437_TandyNew_Mono8pt7b
#define __FONT_SMALL Px437_IBM_BIOS4pt7b
#define FONT_Y_OFFSET 6

static const char* TAG = "MRMEESEEKS::GFX";

#define SLICE_HEIGHT (LCD_HEIGHT / 8)
#define SLICE_SIZE (LCD_WIDTH * SLICE_HEIGHT * 2)
#define VBUF_SIZE (SLICE_SIZE * 5)

// DMA capable vbuf for fast reads/writes from SD
static void* m_dma_vbuf;

// DMA capable buffer for very fast reads/writes, but only 1/4 of the screen
static void* m_dma_buffer;

// Non-DMA capable full screen buffer to reduce flicker
static color_565_t* m_screen_buffer;

// Stop playing bling flag
volatile bool m_stop = false;

// Pause bling flag
volatile bool m_pause = false;

// Current font and color
static color_565_t m_color = COLOR_WHITE;
static color_565_t m_bg_color = COLOR_BLACK;
static bool m_bg_transparent = false;

static font_t m_font = font_medium;
static bool m_wrap = true;

// Cursor coordinates and window
static cursor_coord_t m_cursor = {0, 0};
static area_t m_cursor_area = {0, 0, 0, 0};

// Semaphore to keep bling and gfx from smashing each other
static SemaphoreHandle_t m_mutex;

static inline GFXfont __get_font();

static void __char_bounds(char c,
                          int16_t* x,
                          int16_t* y,
                          int16_t* minx,
                          int16_t* miny,
                          int16_t* maxx,
                          int16_t* maxy) {
  GFXfont font = __get_font();

  if (c == '\n') {  // Newline?
    *x = 0;         // Reset x to zero, advance y by one line
    *y += font.yAdvance;
  } else if (c != '\r') {  // Not a carriage return; is normal char
    uint8_t first = font.first;
    uint8_t last = font.last;
    if ((c >= first) && (c <= last)) {  // Char present in this font?
      GFXglyph* glyph = &font.glyph[c - first];
      uint8_t gw = glyph->width;
      uint8_t gh = glyph->height;
      uint8_t xa = glyph->xAdvance;
      int8_t xo = glyph->xOffset;
      int8_t yo = glyph->yOffset;
      if (m_wrap && ((*x + ((int16_t)xo + gw)) > m_cursor_area.xe)) {
        *x = m_cursor_area.xs;  // Reset x to zero, advance y by one line
        *y += (int16_t)font.yAdvance;
        ESP_LOGD(TAG, "%s wrap", __func__);
      }
      int16_t x1 = *x + xo;
      int16_t y1 = *y + yo;
      int16_t x2 = x1 + gw - 1;
      int16_t y2 = y1 + gh - 1;
      if (x1 < *minx)
        *minx = x1;
      if (y1 < *miny)
        *miny = y1;
      if (x2 > *maxx)
        *maxx = x2;
      if (y2 > *maxy)
        *maxy = y2;
      *x += xa;
    }
  }
}

static inline GFXfont __get_font() {
  GFXfont font = __FONT_SMALL;
  if (m_font == font_large) {
    font = __FONT_LARGE;
  } else if (m_font == font_medium) {
    font = __FONT_MEDIUM;
  } else if (m_font == font_x_large) {
    font = __FONT_X_LARGE;
  }
  return font;
}

/**
 * Draw single character anywhere on the screen. Adapted from Adafruit GFX
 * library.
 * Only supports custom fonts.
 */
static void __draw_char(int16_t x,
                        int16_t y,
                        unsigned char c,
                        color_565_t color,
                        GFXfont font) {
  // Character is assumed previously filtered by write() to eliminate
  // newlines, returns, non-printable characters, etc.  Calling drawChar()
  // directly with 'bad' characters of font may cause mayhem!
  c -= font.first;
  GFXglyph* glyph = &(font.glyph[c]);
  uint8_t* bitmap = font.bitmap;

  uint16_t bo = glyph->bitmapOffset;
  uint8_t w = glyph->width;
  uint8_t h = glyph->height;

  int8_t xo = glyph->xOffset;
  int8_t yo = glyph->yOffset;
  uint8_t xx, yy, bits = 0, bit = 0;

  for (yy = 0; yy < h; yy++) {
    for (xx = 0; xx < w; xx++) {
      if (!(bit++ & 7)) {
        bits = bitmap[bo++];
      }
      if (bits & 0x80) {
        int16_t xxx = xo + x + xx;
        int16_t yyy = yo + y + yy;

        // Crop the text out of the cursor area
        if (xxx >= m_cursor_area.xs && xxx < m_cursor_area.xe &&
            yyy >= m_cursor_area.ys && yyy < m_cursor_area.ye) {
          gfx_draw_pixel(xo + x + xx, yo + y + yy, color);
        }
      }
      bits <<= 1;
    }
  }
}

/**
 * Adapted from Adafruit GFX library. Draws custom font to the screen
 * at the current cursor position.
 */
static void __write_char(uint8_t c, GFXfont font) {
  // If newline, move down a row
  if (c == '\n') {
    m_cursor.x = m_cursor_area.xs;
    m_cursor.y += font.yAdvance;
  }
  // Otherwise, print the character (ignoring carriage return)
  else if (c != '\r') {
    uint8_t first = font.first;

    // Valid char?
    if ((c >= first) && (c <= font.last)) {
      uint8_t c2 = c - first;
      GFXglyph* glyph = &(font.glyph[c2]);

      uint8_t w = glyph->width;
      uint8_t h = glyph->height;

      if ((w > 0) && (h > 0)) {  // Is there an associated bitmap?
        int16_t xo = glyph->xOffset;

        if ((m_cursor.x + (xo + w)) >= m_cursor_area.xe && m_wrap) {
          // Drawing character would go off right edge; wrap to new line
          m_cursor.x = m_cursor_area.xs;
          m_cursor.y += font.yAdvance;
        }

        __draw_char(m_cursor.x, m_cursor.y, c, m_color, font);
      }
      m_cursor.x += glyph->xAdvance;
    }
  }
}

/**
 * @brief Task that runs in the background to invert the screen if tilted a
 * certain amount
 * @param params : parameters passed to the task
 */
static void __invert_screen_task(void* params) {
  while (1) {
    if (state_tilt_get()) {
      if (post_state_get()->accelerometer_ack) {
        accel_axis_t axis = drv_lis2de12_get();

        // If it's already inverted attempt to switch to normal orientation
        if (drv_ili9225_is_inverted()) {
          if (axis.x < (0 - ROTATE_THRESHOLD)) {
            ESP_LOGD(TAG, "Waiting to invert");

            // Flag to bling it should pause
            gfx_pause(true);
            // Wait for bling to release mutex
            xSemaphoreTake(m_mutex, portMAX_DELAY);
            ESP_LOGD(TAG, "Inverting");
            // Invert
            drv_ili9225_invert(false);
            // Free mutex
            xSemaphoreGive(m_mutex);
            // Push the screen to force an update
            gfx_push_screen_buffer();
            // Flag bling it's safe to continue
            gfx_pause(false);
          }
        } else {
          if (axis.x > ROTATE_THRESHOLD) {
            ESP_LOGD(TAG, "Waiting to invert");

            // Flag to bling it should pause
            gfx_pause(true);
            // Wait for bling to release mutex
            xSemaphoreTake(m_mutex, portMAX_DELAY);
            ESP_LOGD(TAG, "Inverting");
            // Invert
            drv_ili9225_invert(true);
            // Free mutex
            xSemaphoreGive(m_mutex);
            // Push the screen to force an update
            gfx_push_screen_buffer();
            // Flag bling it's safe to continue
            gfx_pause(false);
          }
        }
      }
    }

    DELAY(200);
  }
}

void gfx_bg_color_set(color_565_t color) {
  m_bg_color = color;
}

void gfx_bg_transparent(bool transparent) {
  m_bg_transparent = transparent;
}

void gfx_color_set(color_565_t color) {
  m_color = color;
}

void gfx_cursor_area_reset() {
  m_cursor_area.xs = 0;
  m_cursor_area.xe = LCD_WIDTH;
  m_cursor_area.ys = 0;
  m_cursor_area.ye = LCD_HEIGHT;
}

void gfx_cursor_area_set(area_t area) {
  m_cursor_area = area;
}

/**
 * @brief Get the current cursor
 */
cursor_coord_t gfx_cursor_get() {
  return m_cursor;
}

void gfx_cursor_set(cursor_coord_t cursor) {
  m_cursor = cursor;
  m_cursor.y += gfx_font_height() - 2;
}

void gfx_draw_circle(int16_t x, int16_t y, int16_t r, color_565_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t xx = 0;
  int16_t yy = r;

  gfx_draw_pixel(x, y + r, color);
  gfx_draw_pixel(x, y - r, color);
  gfx_draw_pixel(x + r, y, color);
  gfx_draw_pixel(x - r, y, color);

  while (xx < yy) {
    if (f >= 0) {
      yy--;
      ddF_y += 2;
      f += ddF_y;
    }
    xx++;
    ddF_x += 2;
    f += ddF_x;

    gfx_draw_pixel(x + xx, y + yy, color);
    gfx_draw_pixel(x - xx, y + yy, color);
    gfx_draw_pixel(x + xx, y - yy, color);
    gfx_draw_pixel(x - xx, y - yy, color);
    gfx_draw_pixel(x + yy, y + xx, color);
    gfx_draw_pixel(x - yy, y + xx, color);
    gfx_draw_pixel(x + yy, y - xx, color);
    gfx_draw_pixel(x - yy, y - xx, color);
  }
}

void gfx_draw_line(int16_t x0,
                   int16_t y0,
                   int16_t x1,
                   int16_t y1,
                   color_565_t color) {
  int16_t steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    SWAP_INT16_T(x0, y0);
    SWAP_INT16_T(x1, y1);
  }

  // Simplify direction we're drawing the line in
  if (x0 > x1) {
    SWAP_INT16_T(x0, x1);
    SWAP_INT16_T(y0, y1);
  }

  int16_t dx, dy;
  dx = x1 - x0;
  dy = abs(y1 - y0);

  int16_t err = dx / 2;
  int16_t ystep;

  if (y0 < y1) {
    ystep = 1;
  } else {
    ystep = -1;
  }

  for (; x0 <= x1; x0++) {
    if (steep) {
      gfx_draw_pixel(y0, x0, color);
    } else {
      gfx_draw_pixel(x0, y0, color);
    }
    err -= dy;
    if (err < 0) {
      y0 += ystep;
      err += dx;
    }
  }
}

/**
 * Draw raw from memory. Anything that extends outside screen dimensions will be
 * clipped. Care should be
 * taken to ensure width and height are accurate as this will just print data
 * directly from memory. DGAF.
 *
 * @param x		X coordinate on screen to start drawing at
 * @param y 	Y coordinate on screen to start drawing at
 * @param w		Width of the raw data CAREFUL!
 * @param h		Height of the raw data CAREFUL!
 * @param p_raw	Pointer to raw data in memory
 */
void gfx_draw_raw(int16_t x,
                  int16_t y,
                  uint16_t w,
                  uint16_t h,
                  color_565_t* p_raw) {
  for (uint16_t i = 0; i < w; i++) {
    int16_t xx = x + i;
    for (uint16_t j = 0; j < h; j++) {
      int16_t yy = y + j;

      // If in valid range, draw the raw
      if (xx >= 0 || yy >= 0 || xx < LCD_WIDTH || yy < LCD_HEIGHT) {
        gfx_draw_pixel(xx, yy, p_raw[(j * w) + i]);
      }
    }
  }
}

void gfx_draw_raw_file(const char* filename,
                       int16_t x,
                       int16_t y,
                       uint16_t w,
                       uint16_t h,
                       bool transparent,
                       color_565_t transparent_color) {
  FILE* p_raw_file;
  size_t read_bytes = 0;
  int32_t bytecount;
  uint32_t fsize = util_file_size(filename);

  // If file size is 0 then the file does not exist
  if (fsize == 0) {
    ESP_LOGE(TAG, "Could not stat %s.", filename);
    return;
  }

  // Open requested file on SD card
  p_raw_file = fopen(filename, "r");
  // Make sure the file was actually opened
  if (p_raw_file == NULL) {
    ESP_LOGE(TAG, "Could not open '%s'.", filename);
    return;
  }

  void* buffer = heap_caps_malloc(SLICE_SIZE, MALLOC_CAP_SPIRAM);
  void* vbuf = heap_caps_malloc(SLICE_SIZE, MALLOC_CAP_SPIRAM);

  // Setup a file buffer for moar bling
  setvbuf(p_raw_file, vbuf, _IOFBF, SLICE_SIZE);

  bytecount = w * h * 2;
  int xx = x;
  int yy = y;

  if (bytecount != fsize) {
    ESP_LOGE(TAG, "Invalid raw file dimensions. Skipping '%s'", filename);
    free(buffer);
    free(vbuf);
    fclose(p_raw_file);
    return;
  }

  // Blast data to TFT
  while (bytecount > 0) {
    // Populate the row buffer
    read_bytes = fread(buffer, 1, SLICE_SIZE, p_raw_file);
    if (read_bytes == 0) {
      ESP_LOGE(TAG, "Corrupted data while reading '%s' [%s]", filename,
               __func__);
      break;
    }

    color_565_t* ptr = buffer;

    for (size_t i = 0; i < read_bytes; i += 2) {
      color_565_t c = *(ptr++);

      // Crop
      if (xx >= 0 && xx < LCD_WIDTH && yy >= 0 && yy < LCD_HEIGHT) {
        if (!transparent || c != transparent_color) {
          gfx_draw_pixel(xx, yy, UINT16_T_SWAP(c));
        }
      }
      xx++;
      // Wrap
      if (xx >= x + w) {
        xx = x;
        yy++;
      }
    }

    bytecount -= read_bytes;
  }

  free(buffer);
  free(vbuf);
  fclose(p_raw_file);
}

bool gfx_draw_raw_frame(const char* filename,
                        uint16_t frame,
                        int16_t x,
                        int16_t y,
                        uint16_t w,
                        uint16_t h,
                        bool transparent,
                        color_565_t transparent_color) {
  uint16_t frames = 1;
  FILE* p_raw_file;
  size_t read_bytes = 0;
  uint32_t bytecount = w * h * 2;
  uint32_t fsize = util_file_size(filename);
  bool result = false;

  // If file size is 0 then the file does not exist
  if (fsize == 0) {
    ESP_LOGE(TAG, "Could not stat %s.", filename);
    return result;
  }

  // Determine how many frames are in the file, minimum of 1
  frames = MAX(fsize / w / h / 2, 1);

  // Ensure frame we're drawing is valid
  frame = frame % frames;

  // Open requested file on SD card
  p_raw_file = fopen(filename, "r");
  // Make sure the file was actually opened
  if (p_raw_file == NULL) {
    ESP_LOGE(TAG, "Could not open '%s'.", filename);
    return result;
  }

  void* buffer = heap_caps_malloc(SLICE_SIZE, MALLOC_CAP_SPIRAM);
  void* vbuf = heap_caps_malloc(SLICE_SIZE, MALLOC_CAP_SPIRAM);

  // Setup a file buffer for moar bling
  setvbuf(p_raw_file, vbuf, _IOFBF, SLICE_SIZE);

  int xx = x;
  int yy = y;

  // Seek to the frame
  fseek(p_raw_file, frame * bytecount, SEEK_SET);

  // Assume success
  result = true;

  // Blast data to TFT
  while (bytecount > 0) {
    // Populate the row buffer
    read_bytes = fread(buffer, 1, SLICE_SIZE, p_raw_file);

    // Make sure we got the bytes we're expecting
    if (read_bytes != SLICE_SIZE) {
      ESP_LOGE(TAG, "Unable to draw raw frame %d of '%s' ferror=%d", frame,
               filename, ferror(p_raw_file));
      result = false;
      break;
    }

    color_565_t* ptr = buffer;

    for (size_t i = 0; i < read_bytes; i += 2) {
      // Crop
      if (xx >= 0 && xx < LCD_WIDTH && yy >= 0 && yy < LCD_HEIGHT) {
        color_565_t c = *(ptr++);
        if (!transparent || c != transparent_color) {
          //					ESP_LOGD(TAG, "Drawing %#02x
          // pixel
          // at
          //%d,%d
          // Ptr=%#08x", c, xx, yy, (uint32_t)ptr);
          gfx_draw_pixel(xx, yy, UINT16_T_SWAP(c));
        }
      }
      xx++;
      // Wrap
      if (xx >= x + w) {
        xx = x;
        yy++;
      }
    }

    bytecount -= read_bytes;
  }

  free(buffer);
  fclose(p_raw_file);
  return result;
}

inline void gfx_draw_pixel(int16_t x, int16_t y, color_565_t color) {
  // Don't draw off screen
  if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) {
    return;
  }

  int32_t index = (y * LCD_WIDTH) + x;
  // Only draw on screen
  if (index >= 0 && index < (LCD_WIDTH * LCD_HEIGHT)) {
    m_screen_buffer[index] = UINT16_T_SWAP(color);
  }
}

void gfx_draw_rect(int16_t x,
                   int16_t y,
                   int16_t w,
                   int16_t h,
                   color_565_t color) {
  // Top
  gfx_draw_line(x, y, x + w - 1, y, color);
  // Left
  gfx_draw_line(x, y, x, y + h - 1, color);
  // Right
  gfx_draw_line(x + w - 1, y, x + w - 1, y + h - 1, color);
  // Bottom
  gfx_draw_line(x, y + h - 1, x + w - 1, y + h - 1, color);
}

/**
 * @brief Draw a triangle to the screen
 * @param x0 : x coordinate of first vertice
 * @param y0 : y coordinate of first vertice
 * @param x1 : x coordinate of second vertice
 * @param y1 : y coordinate of second vertice
 * @param x2 : x coordinate of third vertice
 * @param y2 : y coordinate of third vertice
 */
void gfx_draw_triangle(int16_t x0,
                       int16_t y0,
                       int16_t x1,
                       int16_t y1,
                       int16_t x2,
                       int16_t y2,
                       uint16_t color) {
  gfx_draw_line(x0, y0, x1, y1, color);
  gfx_draw_line(x1, y1, x2, y2, color);
  gfx_draw_line(x2, y2, x0, y0, color);
}

void gfx_error(char* message) {
  gfx_fill_screen(COLOR_RED);
  gfx_font_set(font_medium);
  gfx_cursor_set((cursor_coord_t){0, 0});
  gfx_color_set(COLOR_BLACK);
  gfx_print(message);
  gfx_push_screen_buffer();
  btn_wait();
  DELAY(200);
  btn_clear();
}

/**
 * Draw a filled circle. Adapted from Adafruit GFX library.
 *
 * https://github.com/adafruit/Adafruit-GFX-Library/blob/master/Adafruit_GFX.cpp
 */
void gfx_fill_circle(int16_t x0, int16_t y0, int16_t r, color_565_t color) {
  int16_t f = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x = 0;
  int16_t y = r;

  gfx_draw_line(x0, y0 - r, x0, y0 - r + 2 * r + 1, color);

  while (x < y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f += ddF_y;
    }
    x++;
    ddF_x += 2;
    f += ddF_x;

    gfx_draw_line(x0 + x, y0 - y, x0 + x, y0 - y + 2 * y + 1, color);
    gfx_draw_line(x0 + y, y0 - x, x0 + y, y0 - x + 2 * x + 1, color);
    gfx_draw_line(x0 - x, y0 - y, x0 - x, y0 - y + 2 * y + 1, color);
    gfx_draw_line(x0 - y, y0 - x, x0 - y, y0 - x + 2 * x + 1, color);
  }
}

void gfx_fill_rect(int16_t x,
                   int16_t y,
                   int16_t w,
                   int16_t h,
                   color_565_t color) {
  for (int16_t xx = x; xx < x + w; xx++) {
    for (int16_t yy = y; yy < y + h; yy++) {
      gfx_draw_pixel(xx, yy, color);
    }
  }
}

void gfx_fill_screen(color_565_t color) {
  for (uint32_t i = 0; i < (LCD_WIDTH * LCD_HEIGHT); i++) {
    m_screen_buffer[i] = UINT16_T_SWAP(color);
  }
}

uint8_t gfx_font_height() {
  return __get_font().yAdvance;
}

/**
 * @brief Get the current font
 */
font_t gfx_font_get() {
  return m_font;
}

/**
 * @brief Set the current font. Do this before measuring text bounds or moving
 * the cursor to be the most accurate
 */
void gfx_font_set(font_t font) {
  m_font = font;
}

/**
 * @brief Returns the width of a character for the current font, careful,  this
 * is unreliable with non-fixed fonts
 * @return Width of a single character in the font
 */
uint8_t gfx_font_width() {
  GFXfont font = __get_font();
  return font.glyph[0].xAdvance;
}

/**
 * @brief Get the current word wrap setting
 * @return Current wrap setting
 */
bool gfx_font_wrap_get() {
  return m_wrap;
}

/**
 * @brief Determine if font shouold wrap or not
 * @param wrap : True to wrap text
 */
void gfx_font_wrap_set(bool wrap) {
  m_wrap = wrap;
}

/**
 * @brief Count frames in a given raw file
 * @param filename : file to count frames in
 * @param w : width of raw file
 * @param h : height of raw file
 * @return Count of frames in the file
 */
uint32_t gfx_frame_count(const char* filename, uint16_t w, uint16_t h) {
  uint32_t fsize = util_file_size(filename);
  if (fsize == 0) {
    ESP_LOGE(TAG, "Could not stat %s.", filename);
    return 0;
  }

  // Determine how many frames are in the file, minimum of 1
  return MAX(fsize / w / h / 2, 1);
}

void gfx_init() {
  gfx_cursor_area_reset();
  util_heap_stats_dump();

  // Allocate some DMA space for silky smooth bling
  m_dma_vbuf = heap_caps_malloc(VBUF_SIZE, MALLOC_CAP_DMA);
  if (m_dma_vbuf == NULL) {
    ESP_LOGE(TAG,
             "Unable to allocate DMA-capable vbuf heap. Falling back to normal "
             "hep. Bling will be impacted.");
    m_dma_vbuf = util_heap_alloc_ext(VBUF_SIZE);
  }
  m_dma_buffer = heap_caps_malloc(SLICE_SIZE, MALLOC_CAP_DMA);
  if (m_dma_buffer == NULL) {
    ESP_LOGE(TAG,
             "Unable to allocate DMA-capable buffer heap. Falling back to "
             "norml heap. Bling will be impacted.");
    m_dma_buffer = util_heap_alloc_ext(SLICE_SIZE);
  }

  // Alloc some space for full sceen buffer
  m_screen_buffer = (color_565_t*)util_heap_alloc_ext(LCD_WIDTH * LCD_HEIGHT *
                                                      sizeof(color_565_t));
  memset(m_screen_buffer, 0, LCD_WIDTH * LCD_HEIGHT * sizeof(color_565_t));

  // Setup mutex
  m_mutex = xSemaphoreCreateMutex();

  // Setup font
  gfx_bg_transparent(true);
  gfx_color_set(COLOR_GREEN);
  gfx_bg_color_set(COLOR_BLACK);
}

/**
 * @brief Start background task to handle inverting screen
 */
void gfx_invert_screen_task_start() {
  ESP_LOGD(TAG, "Starting screen inversion task");
  static StaticTask_t task;
  util_task_create(__invert_screen_task, "Screen Invert", 3000, NULL,
                   TASK_PRIORITY_LOW, &task);
}

/**
 * Load data from a file into memory as a raw image.
 *
 * @param raw		Pointer to where to store the raw 565 data
 * @param filename	Full path to file to read
 * @param size		Size of the buffer
 */
void gfx_load_raw(color_565_t* raw, char* filename, uint32_t size) {
  FILE* raw_file = fopen(filename, "r");
  if (raw_file == NULL) {
    ESP_LOGE(TAG, "%s Could not open '%s'.", __func__, filename);
    return;
  }
  uint32_t fsize = util_file_size(filename);
  uint32_t bytes_to_read = MIN(fsize, size * 2);

  size_t bytes_read = fread(raw, 1, bytes_to_read, raw_file);
  if (bytes_read == 0) {
    ESP_LOGE(TAG, "%s Could not read '%s'.", __func__, filename);
  }

  fclose(raw_file);

  // Reverse byte order
  for (uint32_t i = 0; i < size; i++) {
    raw[i] = UINT16_T_SWAP(raw[i]);
  }
}

/**
 * @brief Set pause flag to temporarily pause bling
 * @param pause : true to pause bling false to unpause
 */
void gfx_pause(bool pause) {
  m_pause = pause;
}

void gfx_play_gif_file(char* filename) {
  int err = 0;

  if (!drv_sd_mounted()) {
    char buffer[256];
    sprintf(buffer, "Unable to execute '%s', no SD card inserted.", filename);
    ESP_LOGE(TAG, "%s", buffer);
    gfx_error(buffer);
    return;
  }

  // Open the file
  GifFileType* p_gif = DGifOpenFileName(filename, &err);
  //  if (err == GIF_ERROR) {
  //    ESP_LOGE(TAG, "Unable to play %s", filename);
  //    return;
  //  }

  FILE* p_file = ((GifFilePrivateType*)(p_gif->Private))->File;
  // Allocate some DMA buffer
  char* vbuf = (char*)heap_caps_malloc(VBUF_SIZE, MALLOC_CAP_DMA);
  setvbuf(p_file, vbuf, _IOFBF, VBUF_SIZE);

  GifRecordType record;
  SavedImage* p_image;
  void* line_buffer = NULL;
  long first_frame_pos = 0;
  bool first_frame_found = false;

  ESP_LOGD(
      TAG,
      "Width = %d\tHeight = %d\tDepth = %d\tBackground = %d\tImage Count = %d",
      p_gif->SWidth, p_gif->SHeight, p_gif->SColorResolution,
      p_gif->SBackGroundColor, p_gif->ImageCount);

  if (p_gif->SWidth != LCD_WIDTH || p_gif->SHeight != LCD_HEIGHT) {
    ESP_LOGE(TAG, "'%s' is not %dx%d", filename, LCD_WIDTH, LCD_HEIGHT);
    goto gfx_play_gif_file_cleanup;
  }

  // Allocate a buffer for a full frame
  line_buffer =
      util_heap_alloc_ext(LCD_HEIGHT * LCD_WIDTH * sizeof(GifPixelType));
  memset(line_buffer, 0, LCD_HEIGHT * LCD_WIDTH * sizeof(GifPixelType));

  uint32_t start = MILLIS();
  do {
    if (!first_frame_found) {
      first_frame_pos = ftell(p_file);
    }

    err = DGifGetRecordType(p_gif, &record);
    if (err == GIF_ERROR) {
      // ESP_LOGE(TAG, "Error reading record [%d]", p_gif->Error);
    }

    // Handle GIF Records
    switch (record) {
      case IMAGE_DESC_RECORD_TYPE:;

        // ESP_LOGD(TAG, "IMAGE_DESC start Free Heap: %d, Lowest Free Heap: %d",
        // xPortGetFreeHeapSize(),
        //          xPortGetMinimumEverFreeHeapSize());
        if (!first_frame_found) {
          first_frame_found = true;
        }

        // Grab the image description data
        if (DGifGetImageDesc(p_gif) == GIF_ERROR) {
          ESP_LOGE(TAG, "Unable to read Image Desc [%d]", p_gif->Error);
          break;
        }

        p_image = &p_gif->SavedImages[p_gif->ImageCount - 1];
        /* Allocate memory for the image */
        if (p_image->ImageDesc.Width < 0 && p_image->ImageDesc.Height < 0 &&
            p_image->ImageDesc.Width > (INT_MAX / p_image->ImageDesc.Height)) {
          ESP_LOGE(TAG, "Unable to allocate image memory");
          return;
        }

        uint16_t w = p_image->ImageDesc.Width;
        uint16_t h = p_image->ImageDesc.Height;
        uint16_t x = p_image->ImageDesc.Left;
        uint16_t y = p_image->ImageDesc.Top;

        int err = DGifGetLine(p_gif, line_buffer, w * h);
        if (err == GIF_ERROR) {
          ESP_LOGE(TAG, "unable to read GIF line, err=%d", p_gif->Error);
          return;
        }

        for (uint8_t row = 0; row < h; row++) {
          for (uint8_t col = 0; col < w; col++) {
            uint8_t* z = (uint8_t*)line_buffer + (row * w) + col;
            if (*z != p_gif->SBackGroundColor) {
              GifColorType color;
              // if ( p_gif->SavedImages[p_gif->ImageCount-1].ImageDesc.ColorMap
              // != NULL) {
              //   ESP_LOGD(TAG, "Using saved image color map :)");
              //   color =
              //   p_gif->SavedImages[p_gif->ImageCount-1].ImageDesc.ColorMap->Colors[*z];
              // } else {
              //   ESP_LOGD(TAG, "Using global color map");
              color = p_gif->SColorMap->Colors[*z];
              // }
              color_565_t c =
                  util_rgb_to_565_discreet(color.Blue, color.Green, color.Red);
              gfx_draw_pixel(x + col, y + row, c);
            }
          }
        }

        // Push the frame buffer
        gfx_push_screen_buffer();
        // ESP_LOGD(TAG, "IMAGE DESC END Free Heap: %d, Lowest Free Heap: %d",
        // xPortGetFreeHeapSize(),
        //          xPortGetMinimumEverFreeHeapSize());
        break;

      /*******
       * Handle Extension records
       *******/
      case EXTENSION_RECORD_TYPE:
        // if (DGifGetExtension(p_gif, &ext_function, &p_ext_data) == GIF_ERROR)
        //   return (GIF_ERROR);
        // /* Create an extension block with our data */
        // if (p_ext_data != NULL) {
        //   if (GifAddExtensionBlock(&p_gif->ExtensionBlockCount,
        //                            &p_gif->ExtensionBlocks, ext_function,
        //                            p_ext_data[0], &p_ext_data[1]) ==
        //                            GIF_ERROR)
        //     return (GIF_ERROR);
        // }
        // while (p_ext_data != NULL) {
        //   if (DGifGetExtensionNext(p_gif, &p_ext_data) == GIF_ERROR)
        //     return (GIF_ERROR);
        //   /* Continue the extension block */
        //   if (p_ext_data != NULL)
        //     if (GifAddExtensionBlock(
        //             &p_gif->ExtensionBlockCount, &p_gif->ExtensionBlocks,
        //             CONTINUE_EXT_FUNC_CODE, p_ext_data[0], &p_ext_data[1]) ==
        //             GIF_ERROR)
        //       return (GIF_ERROR);
        // }
        break;

      /*****
       * Screen description
       *****/
      case SCREEN_DESC_RECORD_TYPE:
        break;

      /****
       * Final record in the GIF
       ****/
      case TERMINATE_RECORD_TYPE:;
        uint32_t delta = MILLIS() - start;
        ESP_LOGD(TAG, "Complete. Frame count: %d Total time: %d ms %2.2f FPS",
                 p_gif->ImageCount, delta,
                 (float)p_gif->ImageCount / ((float)delta / 1000.0));
        start = MILLIS();
        util_heap_stats_dump();

        // Rewind file
        fseek(p_file, first_frame_pos, SEEK_SET);

        // Free some data
        GifFreeSavedImages(p_gif);
        p_gif->SavedImages = NULL;
        GifFreeExtensions(&p_gif->ExtensionBlockCount, &p_gif->ExtensionBlocks);
        // Free local color map
        if (p_gif->Image.ColorMap) {
          GifFreeMapObject(p_gif->Image.ColorMap);
          p_gif->Image.ColorMap = NULL;
        }

        // Reset image count
        p_gif->ImageCount = 0;
        util_heap_stats_dump();
        break;
      /****
       * Handle any invalid data. This should quit and clean up
       ****/
      case UNDEFINED_RECORD_TYPE:
        // ESP_LOGD(TAG, "UNDEFINED_RECORD_TYPE");
        break;
    }
    if (btn_state() > 0) {
      DELAY(100);
      btn_clear();
      break;
    }

  } while (1);
  // } while (record != TERMINATE_RECORD_TYPE);

gfx_play_gif_file_cleanup:
  DGifCloseFile(p_gif, &err);
  free(line_buffer);
  free(vbuf);
  ESP_LOGD(TAG, "Closed gif file, err=%d", err);
  util_heap_stats_dump();
}

/**
 * @brief Play a raw file directly to the screen avoiding the buffer
 * @param filename : full path to file to play
 * @param x : x coordinate to place the left most pixel
 * @param y : y coordinate to place the top most pixel
 * @param w : Width of the raw
 * @param h : Height of the raw
 * @callback : Function to run after every frame
 * @param loop : Set to true to loop forever until a button is pressed.
 * Otherwise run once until completion.
 * @param data : Data to pass to the callback
 * @return : Mask of the button that interrupted the raw playback
 */
uint8_t gfx_play_raw_file(const char* filename,
                          int16_t x,
                          int16_t y,
                          uint16_t w,
                          uint16_t h,
                          void (*p_frame_callback)(uint8_t frame, void* p_data),
                          bool loop,
                          void* data) {
  FILE* p_raw_file;

  size_t read_bytes = 0;
  int32_t bytecount = w * h * 2;
  uint16_t frames = 1;
  uint8_t button =
      0;  // Button state, changed when the animation is stopped by the user

  // Counter for bad blocks from SD.
  uint32_t bad_block_counter = 0;

  m_stop = false;
  m_pause = false;

  if (!drv_sd_mounted()) {
    char buffer[256];
    sprintf(buffer, "Unable to execute '%s', no SD card inserted.", filename);
    ESP_LOGE(TAG, "%s", buffer);
    gfx_error(buffer);
    return 0;
  }
  uint32_t fsize = util_file_size(filename);
  if (fsize == 0) {
    ESP_LOGE(TAG, "Could not stat %s.", filename);
    return 0;
  }

  if ((fsize % bytecount) != 0) {
    ESP_LOGE(TAG, "Invalid raw file dimensions. Skipping '%s'", filename);
    return 0;
  }

  // Determine how many frames are in the file, minimum of 1
  frames = MAX(fsize / w / h / 2, 1);

  // Open requested file on SD card
  p_raw_file = fopen(filename, "r");
  if (p_raw_file == NULL) {
    ESP_LOGE(TAG, "Could not open %s.", filename);
    return 0;
  }

  setvbuf(p_raw_file, m_dma_vbuf, _IOFBF, VBUF_SIZE);
  util_heap_stats_dump();

  // Don't smash other graphics
  xSemaphoreTake(m_mutex, portMAX_DELAY);

  // Do not allow inverts while we're painting the display

  do {
    // Ensure we're starting at beginning of file for first frame
    rewind(p_raw_file);

    for (uint16_t i = 0; i < frames; i++) {
      // Hang out here if something pauses bling
      while (m_pause) {
        xSemaphoreGive(m_mutex);
        DELAY(10);
        xSemaphoreTake(m_mutex, portMAX_DELAY);
      }
#ifdef CONFIG_SHOW_FPS
      uint32_t start = MILLIS();
#endif

      // Set TFT address window to clipped image bounds
      drv_ili9225_set_addr(x, y, x + w - 1, y + h - 1);

      bytecount = w * h * 2;

      // Blast data to TFT
      while (bytecount > 0) {
        // Populate the row buffer
        read_bytes = fread(m_dma_buffer, 1, SLICE_SIZE, p_raw_file);

        // If consequetive bad blocks, we need to quit
        if (bad_block_counter > 100) {
          ESP_LOGE(TAG, "Too many failures reading SD card. Quitting bling.");
          m_stop = true;
          break;
        }

        // fread will return 0 if there was an error in underlying fatfs, try to
        // recover from it by skipping that frame
        if (read_bytes == 0) {
          bad_block_counter++;
          ESP_LOGE(TAG, "Bad block encountered. Skipping frame. %d/100",
                   bad_block_counter);
          break;
        } else {
          bad_block_counter = 0;
        }

        // Push the colors async
        drv_ili9225_push_colors(m_dma_buffer, read_bytes);

        bytecount -= read_bytes;
      }

      // frame complete, callback
      if (p_frame_callback != NULL) {
        p_frame_callback(i, data);
      }

      // if we're looping give them a way out
      if ((btn_state() > 0 && loop) || m_stop) {
        loop = false;
        button = btn_state();
        break;  // Quit the for loop
      }

      DELAY(1);
      esp_task_wdt_reset();

#ifdef CONFIG_SHOW_FPS
      if ((i % 10) == 0) {
        uint32_t delta = MILLIS() - start;
        ESP_LOGD(TAG, "FPS:%.2f Delta:%d", (float)(1000.0 / (float)delta),
                 (int)delta);
      }

#endif
    }

  } while (loop && !m_stop);

  fclose(p_raw_file);

  // Done with mutex
  xSemaphoreGive(m_mutex);

  // All done, if looping button state will the button that caused it to quit,
  // otherwise 0
  return button;
}

/**
 * @brief Print text at the current cursor
 * @param text : the text to print
 */
void gfx_print(const char* text) {
  GFXfont font = __get_font();

  // Draw the background
  if (!m_bg_transparent) {
    int16_t x, y;
    uint16_t w, h;

    gfx_text_bounds(text, m_cursor.x, m_cursor.y, &x, &y, &w, &h);
    gfx_fill_rect(x, y, w, h, m_bg_color);
  }

  for (uint16_t i = 0; i < strlen(text); i++) {
    __write_char(text[i], font);
  }
}

/**
 * @brief Print text with a drop shadow
 *
 * @param text : Text to print
 * @param shadow : Color to use for the drop shadow
 * @param cursor : Cursor to use to draw text
 */
void gfx_print_drop_shadow(const char* text,
                           color_565_t shadow,
                           cursor_coord_t cursor) {
  color_565_t color = m_color;  // save current color
  gfx_color_set(shadow);
  gfx_cursor_set(cursor);
  gfx_print(text);

  // Return to original position and color
  cursor.x -= 2;  // Offset by a bit
  cursor.y -= 2;
  gfx_cursor_set(cursor);
  gfx_color_set(color);
  gfx_print(text);
}

/**
 * @brief Push the back buffer to the display
 */
void gfx_push_screen_buffer() {
  uint32_t bytes_sent = 0;
  uint32_t bytes_to_send = LCD_WIDTH * LCD_HEIGHT * sizeof(color_565_t);

  xSemaphoreTake(m_mutex, portMAX_DELAY);
  drv_ili9225_set_addr(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);

  while (bytes_sent < bytes_to_send) {
    memcpy(m_dma_buffer, (void*)m_screen_buffer + bytes_sent, SLICE_SIZE);
    drv_ili9225_push_colors(m_dma_buffer, SLICE_SIZE);
    bytes_sent += SLICE_SIZE;
  }
  xSemaphoreGive(m_mutex);
}

/**
 * @brief Copy the screen buffer into a secondary buffer for other uses
 *
 * @param buffer :	Pointer to buffer to copy into. It must be LCD_WIDTH *
 * LCD_HEIGHT * sizeof(color_565_t)
 */
void gfx_screen_buffer_copy(void* buffer) {
  memcpy(buffer, m_screen_buffer, LCD_WIDTH * LCD_HEIGHT * sizeof(color_565_t));
}

/**
 * @brief Restore a screen buffer copy back into the screen buffer
 *
 * @param buffer :	Pointer to buffer to copy from. It must be LCD_WIDTH *
 * LCD_HEIGHT * sizeof(color_565_t)
 */
void gfx_screen_buffer_restore(void* buffer) {
  memcpy(m_screen_buffer, buffer,
         (LCD_WIDTH * LCD_HEIGHT * sizeof(color_565_t)));
}

/**
 * Stop whatever animation is running (if any)
 */
void gfx_stop() {
  m_stop = true;
}

void gfx_text_bounds(const char* str,
                     int16_t x,
                     int16_t y,
                     int16_t* x1,
                     int16_t* y1,
                     uint16_t* w,
                     uint16_t* h) {
  uint8_t c;  // Current character

  *x1 = x;
  *y1 = y;
  *w = *h = 0;

  int16_t minx = m_cursor_area.xe;
  int16_t miny = m_cursor_area.ye;
  int16_t maxx = -1;
  int16_t maxy = -1;

  while ((c = *str++))
    __char_bounds(c, &x, &y, &minx, &miny, &maxx, &maxy);

  if (maxx >= minx) {
    *x1 = minx;
    *w = maxx - minx + 1;
  }
  if (maxy >= miny) {
    *y1 = miny;
    *h = maxy - miny + 1;
  }
}

/**
 * @brief Draw a button on screen
 * @param text : Text to put in the button
 * @param x : Left-most coordinate of button
 * @param y : Right-most coordinate of button
 * @param selected : true if render as a selected button
 */
void gfx_ui_draw_button(const char* text, int16_t x, int16_t y, bool selected) {
  uint16_t w, h;
  uint16_t tw, th;
  cursor_coord_t cursor;
  uint16_t border_color = COLOR_WHITE;

  if (selected) {
    border_color = COLOR_GREEN;
  }

  gfx_font_set(font_small);
  gfx_text_bounds(text, x + 5, y + 5, &cursor.x, &cursor.y, &tw, &th);

  // Determine button width/height
  w = tw + 10;
  h = th + 10;

  // Do some text clipping
  area_t area = {
      .xs = cursor.x,
      .ys = cursor.y,
      .xe = cursor.x + w,
      .ye = cursor.y + h,
  };
  gfx_cursor_area_set(area);

  // Adjust cursor y, dirty hack I know, don't judge me.
  cursor.y += 6;

  gfx_fill_rect(x, y, w, h, COLOR_BLACK);
  gfx_draw_rect(x, y, w, h, border_color);

  gfx_color_set(COLOR_WHITE);
  gfx_cursor_set(cursor);
  gfx_print(text);

  gfx_cursor_area_reset();
}

/**
 * @brief Helper function to draw the button on the left
 *
 * @param text : Text to use in the button
 */
void gfx_ui_draw_left_button(const char* text) {
  int16_t x, y;
  uint16_t w, h;
  gfx_font_set(font_small);
  gfx_text_bounds(text, 0, 0, &x, &y, &w, &h);
  gfx_ui_draw_button(text, x + 5, LCD_HEIGHT - 15 - h, false);
}

/**
 * @brief Helper function to draw the button on the right
 *
 * @param text : Text to use in the button
 */
void gfx_ui_draw_right_button(const char* text) {
  int16_t x, y;
  uint16_t w, h;
  gfx_font_set(font_small);
  gfx_text_bounds(text, 0, 0, &x, &y, &w, &h);
  gfx_ui_draw_button(text, LCD_WIDTH - 15 - w + x, LCD_HEIGHT - 15 - h, false);
}

void gfx_ui_draw_ok_cancel() {
  gfx_ui_draw_button("Cancel", 5, 154, false);
  gfx_ui_draw_button("Ok", 184, 154, false);
}
