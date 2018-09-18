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
#ifndef COMPONENTS_GFX_H_
#define COMPONENTS_GFX_H_

#include <stdint.h>
#include "util.h"

//#define ANX_COLOR			uint16_t

#define COLOR_BLACK 0x0000 /*   0,   0,   0 */
#define COLOR_BROWN 0x9B26
#define COLOR_LIGHTBROWN 0xF5A8
#define COLOR_NAVY 0x000F /*   0,   0, 128 */
#define COLOR_DARKBLUE 0x18E8
#define COLOR_DARKGREEN 0x03E0 /*   0, 128,   0 */
#define COLOR_DARKCYAN 0x03EF  /*   0, 128, 128 */
#define COLOR_DARKRED 0x5800
#define COLOR_MAROON 0x7800      /* 128,   0,   0 */
#define COLOR_PURPLE 0x780F      /* 128,   0, 128 */
#define COLOR_OLIVE 0x7BE0       /* 128, 128,   0 */
#define COLOR_LIGHTBLUE 0xB6FF   /* #B4DEFF */
#define COLOR_LIGHTGREY 0xC618   /* 192, 192, 192 */
#define COLOR_DARKGREY 0x7BEF    /* 128, 128, 128 */
#define COLOR_BLUE 0x001F        /*   0,   0, 255 */
#define COLOR_GOLD 0xFEA0        /* 255, 215,   0 */
#define COLOR_GREEN 0x07E0       /*   0, 255,   0 */
#define COLOR_CYAN 0x07FF        /*   0, 255, 255 */
#define COLOR_RED 0xF800         /* 255,   0,   0 */
#define COLOR_MAGENTA 0xF81F     /* 255,   0, 255 */
#define COLOR_YELLOW 0xFFE0      /* 255, 255,   0 */
#define COLOR_WHITE 0xFFFF       /* 255, 255, 255 */
#define COLOR_ORANGE 0xFD20      /* 255, 165,   0 */
#define COLOR_GREENYELLOW 0xAFE5 /* 173, 255,  47 */
#define COLOR_PINK 0xFB56
#define COLOR_NEONPURPLE 0xFD5F
#define COLOR_WESTERN_YELLOW 0xFD63 /* 255 175 27 */
#define COLOR_WESTERN_ORANGE 0xDBC2 /* 218 122 17 */

typedef enum { font_small, font_medium, font_large, font_x_large } font_t;

/**
 * Struct for holding area X,Y start and end values for the screen
 */
typedef struct {
  int16_t xs, ys, xe, ye;
} area_t;

typedef struct {
  int16_t x;
  int16_t y;
} cursor_coord_t;

extern void gfx_bg_color_set(color_565_t color);
extern void gfx_bg_transparent(bool transparent);
extern void gfx_color_set(color_565_t color);

/**
 * Sets the current color used for printing
 *
 * @param color	Foreground color to use
 */
extern void gfx_color_set(color_565_t color);

/**
 * @brief Get the current cursor
 */
extern cursor_coord_t gfx_cursor_get();

/**
 * Sets the cursor location as the top left coordinate of the text. No need to
 * adjust for font height etc.
 *
 * @param cursor	The coordinate to move the cursor to
 */
extern void gfx_cursor_set(cursor_coord_t);

/**
 * Reset the cursor area to the default value. That is the entire screen.
 */
extern void gfx_cursor_area_reset();

/**
 * Sets the printable area.
 *
 * @param area	The area of the screen in which printing is allowed. Text
 * will wrap if it reaches the right-most X
 * coordinate and should scroll if it reaches the bottom of the Y coordinate
 */
extern void gfx_cursor_area_set(area_t area);
extern void gfx_draw_circle(int16_t x, int16_t y, int16_t r, color_565_t color);
extern void gfx_draw_line(int16_t x0,
                          int16_t y0,
                          int16_t x1,
                          int16_t y1,
                          color_565_t color);
extern void gfx_draw_pixel(int16_t x, int16_t y, color_565_t color);

/**
 * @brief Draw a triangle to the screen
 * @param x0 : x coordinate of first vertice
 * @param y0 : y coordinate of first vertice
 * @param x1 : x coordinate of second vertice
 * @param y1 : y coordinate of second vertice
 * @param x2 : x coordinate of third vertice
 * @param y2 : y coordinate of third vertice
 */
extern void gfx_draw_triangle(int16_t x0,
                              int16_t y0,
                              int16_t x1,
                              int16_t y1,
                              int16_t x2,
                              int16_t y2,
                              uint16_t color);

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
extern uint8_t gfx_play_raw_file(const char* filename,
                                 int16_t x,
                                 int16_t y,
                                 uint16_t w,
                                 uint16_t h,
                                 void (*p_frame_callback)(uint8_t frame,
                                                          void* p_data),
                                 bool loop,
                                 void* data);
extern bool gfx_draw_raw_frame(const char* filename,
                               uint16_t frame,
                               int16_t x,
                               int16_t y,
                               uint16_t w,
                               uint16_t h,
                               bool transparent,
                               color_565_t transparent_color);

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
extern void gfx_draw_raw(int16_t x,
                         int16_t y,
                         uint16_t w,
                         uint16_t h,
                         color_565_t* p_raw);

extern void gfx_draw_raw_file(const char* filename,
                              int16_t x,
                              int16_t y,
                              uint16_t w,
                              uint16_t h,
                              bool transparent,
                              color_565_t transparent_color);
extern void gfx_draw_rect(int16_t x,
                          int16_t y,
                          int16_t w,
                          int16_t h,
                          color_565_t color);

/**
 * Print error message to screen and wait for button
 *
 * @param message	Message to print to screen.
 */
extern void gfx_error(char* message);

extern void gfx_fill_circle(int16_t x0,
                            int16_t y0,
                            int16_t r,
                            color_565_t color);
extern void gfx_fill_rect(int16_t x,
                          int16_t y,
                          int16_t w,
                          int16_t h,
                          color_565_t color);
extern void gfx_fill_screen(color_565_t color);
extern uint8_t gfx_font_height();

/**
 * @brief Get the current font
 */
extern font_t gfx_font_get();

/**
 * @brief Set the current font. Do this before measuring text bounds or moving
 * the cursor to be the most accurate
 */
extern void gfx_font_set(font_t font);

/**
 * @brief Returns the width of a character for the current font, careful,  this
 * is unreliable with non-fixed fonts
 * @return Width of a single character in the font
 */
extern uint8_t gfx_font_width();

/**
 * @brief Get the current word wrap setting
 * @return Current wrap setting
 */
extern bool gfx_font_wrap_get();

/**
 * @brief Determine if font shouold wrap or not
 * @param wrap : True to wrap text
 */
extern void gfx_font_wrap_set(bool wrap);

/**
 * @brief Count frames in a given raw file
 * @param filename : file to count frames in
 * @param w : width of raw file
 * @param h : height of raw file
 * @return Count of frames in the file
 */
extern uint32_t gfx_frame_count(const char* filename, uint16_t w, uint16_t h);

extern void gfx_init();

/**
 * @brief Start background task to handle inverting screen
 */
extern void gfx_invert_screen_task_start();

/**
 * Load data from a file into memory as a raw image.
 *
 * @param raw		Pointer to where to store the raw 565 data
 * @param filename	Full path to file to read
 * @param size		Size of the buffer
 */
extern void gfx_load_raw(color_565_t* raw, char* filename, uint32_t size);

/**
 * @brief Set pause flag to temporarily pause bling
 * @param pause : true to pause bling false to unpause
 */
extern void gfx_pause(bool pause);

extern void gfx_play_gif_file();

/**
 * @brief Print text at the current cursor
 * @param text : the text to print
 */
extern void gfx_print(const char* text);

/**
 * @brief Print text with a drop shadow
 *
 * @param text : Text to print
 * @param shadow : Color to use for the drop shadow
 * @param cursor : Cursor to use to draw text
 */
extern void gfx_print_drop_shadow(const char* text,
                                  color_565_t shadow,
                                  cursor_coord_t cursor);

/**
 * @brief Push the back buffer to the display
 */
extern void gfx_push_screen_buffer();

/**
 * @brief Copy the screen buffer into a secondary buffer for other uses
 *
 * @param buffer :	Pointer to buffer to copy into. It must be LCD_WIDTH *
 * LCD_HEIGHT * sizeof(color_565_t)
 */
extern void gfx_screen_buffer_copy(void* buffer);

/**
 * @brief Restore a screen buffer copy back into the screen buffer
 *
 * @param buffer :	Pointer to buffer to copy from. It must be LCD_WIDTH *
 * LCD_HEIGHT * sizeof(color_565_t)
 */
extern void gfx_screen_buffer_restore(void* buffer);

/**
 * Stop whatever animation is running (if any)
 */
extern void gfx_stop();
extern void gfx_text_bounds(const char* str,
                            int16_t x,
                            int16_t y,
                            int16_t* x1,
                            int16_t* y1,
                            uint16_t* w,
                            uint16_t* h);

/**
 * @brief Draw a button on screen
 * @param text : Text to put in the button
 * @param x : Left-most coordinate of button
 * @param y : Right-most coordinate of button
 * @param selected : true if render as a selected button
 */
extern void gfx_ui_draw_button(const char* text,
                               int16_t x,
                               int16_t y,
                               bool selected);

/**
 * @brief Helper function to draw the button on the left
 *
 * @param text : Text to use in the button
 */
extern void gfx_ui_draw_left_button(const char* text);

/**
 * @brief Helper function to draw the button on the right
 *
 * @param text : Text to use in the button
 */
extern void gfx_ui_draw_right_button(const char* text);

/**
 * @brief UI Helper Draw ok / cancel buttons on screen
 */
extern void gfx_ui_draw_ok_cancel();
#endif /* COMPONENTS_GFX_H_ */
