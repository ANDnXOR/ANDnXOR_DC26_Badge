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
#ifndef MAIN_DRV_ILI9341_H_
#define MAIN_DRV_ILI9225_H_

#include <stdint.h>
#include "gfx.h"

//*****************************************************************************
//
// Make sure all of the definitions in this header have a C binding.
//
//*****************************************************************************

#ifdef __cplusplus
extern "C" {
#endif

#define LCD_HEIGHT 176
#define LCD_WIDTH 220
#define LCD_PIXEL_COUNT (LCD_WIDTH * LCD_HEIGHT)
#define LCD_BUFFER_SIZE (LCD_PIXEL_COUNT * sizeof(color_565_t))

/* ILI9225 LCD Registers */
#define ILI9225_DRIVER_OUTPUT_CTRL (0x01u)             // Driver Output Control
#define ILI9225_LCD_AC_DRIVING_CTRL (0x02u)            // LCD AC Driving Control
#define ILI9225_ENTRY_MODE (0x03u)                     // Entry Mode
#define ILI9225_ENTRY_MODE_RGB (0x0000)                // RGB order
#define ILI9225_ENTRY_MODE_BGR (0x1000)                // BGR order
#define ILI9225_ENTRY_MODE_NORMAL (0x0028)             // ID=2 AM=1
#define ILI9225_ENTRY_MODE_NORMAL_MIRRORED (0x0038)    // ID=3 AM=1
#define ILI9225_ENTRY_MODE_INVERTED (0x0018)           // ID=1 AM=1
#define ILI9225_ENTRY_MODE_INVERTED_MIRRORED (0x0008)  // ID=0 AM=1
#define ILI9225_DISP_CTRL1 (0x07u)                     // Display Control 1
#define ILI9225_BLANK_PERIOD_CTRL1 (0x08u)             // Blank Period Control
#define ILI9225_FRAME_CYCLE_CTRL (0x0Bu)               // Frame Cycle Control
#define ILI9225_INTERFACE_CTRL (0x0Cu)                 // Interface Control
#define ILI9225_OSC_CTRL (0x0Fu)                       // Osc Control
#define ILI9225_POWER_CTRL1 (0x10u)                    // Power Control 1
#define ILI9225_POWER_CTRL2 (0x11u)                    // Power Control 2
#define ILI9225_POWER_CTRL3 (0x12u)                    // Power Control 3
#define ILI9225_POWER_CTRL4 (0x13u)                    // Power Control 4
#define ILI9225_POWER_CTRL5 (0x14u)                    // Power Control 5
#define ILI9225_VCI_RECYCLING (0x15u)                  // VCI Recycling
#define ILI9225_RAM_ADDR_SET1 (0x20u)   // Horizontal GRAM Address Set
#define ILI9225_RAM_ADDR_SET2 (0x21u)   // Vertical GRAM Address Set
#define ILI9225_GRAM_DATA_REG (0x22u)   // GRAM Data Register
#define ILI9225_GATE_SCAN_CTRL (0x30u)  // Gate Scan Control Register
#define ILI9225_VERTICAL_SCROLL_CTRL1 \
  (0x31u)  // Vertical Scroll Control 1 Register
#define ILI9225_VERTICAL_SCROLL_CTRL2 \
  (0x32u)  // Vertical Scroll Control 2 Register
#define ILI9225_VERTICAL_SCROLL_CTRL3 \
  (0x33u)  // Vertical Scroll Control 3 Register
#define ILI9225_PARTIAL_DRIVING_POS1 \
  (0x34u)  // Partial Driving Position 1 Register
#define ILI9225_PARTIAL_DRIVING_POS2 \
  (0x35u)  // Partial Driving Position 2 Register
#define ILI9225_HORIZONTAL_WINDOW_ADDR1 \
  (0x36u)  // Horizontal Address Start Position
#define ILI9225_HORIZONTAL_WINDOW_ADDR2 \
  (0x37u)  // Horizontal Address End Position
#define ILI9225_VERTICAL_WINDOW_ADDR1 \
  (0x38u)  // Vertical Address Start Position
#define ILI9225_VERTICAL_WINDOW_ADDR2 (0x39u)  // Vertical Address End Position
#define ILI9225_GAMMA_CTRL1 (0x50u)            // Gamma Control 1
#define ILI9225_GAMMA_CTRL2 (0x51u)            // Gamma Control 2
#define ILI9225_GAMMA_CTRL3 (0x52u)            // Gamma Control 3
#define ILI9225_GAMMA_CTRL4 (0x53u)            // Gamma Control 4
#define ILI9225_GAMMA_CTRL5 (0x54u)            // Gamma Control 5
#define ILI9225_GAMMA_CTRL6 (0x55u)            // Gamma Control 6
#define ILI9225_GAMMA_CTRL7 (0x56u)            // Gamma Control 7
#define ILI9225_GAMMA_CTRL8 (0x57u)            // Gamma Control 8
#define ILI9225_GAMMA_CTRL9 (0x58u)            // Gamma Control 9
#define ILI9225_GAMMA_CTRL10 (0x59u)           // Gamma Control 10

#define ILI9225C_INVOFF 0x20
#define ILI9225C_INVON 0x21

extern void drv_ili9225_backlight_set(uint8_t brightness);

/**
 * @brief Sets if RGB mode should be used.
 * @param rgb : true for RGB false for BGR
 */
extern void drv_ili9225_rgb_mode(bool rgb);

/**
 * @brief Initialize the LCD driver
 */
extern void drv_ili9225_init();

/**
 * @brief Get the current inversion state of the screen. This properly handles
 * the differences between Pickle Rick, Jerry Smith, and Mr Meeseeks hardware
 * revisions
 * @return True if inverted.
 */
extern bool drv_ili9225_is_inverted();
extern void drv_ili9225_invert(bool inverted);
/**
 * @brief Mirror the display
 */
extern void drv_ili9225_mirror(bool mirror);
extern void drv_ili9225_push_colors(uint8_t* p_colors, int32_t size);
extern void drv_ili9225_set_addr(uint16_t x0,
                                 uint16_t y0,
                                 uint16_t x1,
                                 uint16_t y1);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_DRV_ILI9341_H_ */
