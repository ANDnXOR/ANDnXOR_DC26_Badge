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

#define UI_MENU_LABEL_LENGTH_MAX 16

typedef void (*ui_menu_callback_t)(void* data);

typedef struct {
  char* text;
  char* preview;
  ui_menu_callback_t callback;
  void* data;
} ui_menu_item_t;

typedef struct {
  uint8_t count;
  uint8_t top;
  uint8_t selected;
  ui_menu_item_t* items;
} ui_menu_t;

/**
 * @brief Display about screen for the badge
 */
extern void ui_about();

/**
 * @brief Set the allow interrupt flag which tells the UI if we can handle being interrupted by ralph (or any other animation)
 * @param allow : Set to true to allow interrupts false if not
 */
extern void ui_allow_interrupt(bool allow);

/**
 * @brief Use a modal dialog to get confirmation input from the user
 * @param prompt : Main prompt to show
 * @param text : String to print in the dialog
 *
 * @return : 1 if OK, 0 if CANCEL
 */
extern uint8_t ui_confirm(char* prompt, char* text);

/**
 * @brief Get input from the user
 * @param title : The prompt for the user
 * @param buffer : Pointer to buffer of starting value to use for input. Gets
 * replaced by user
 * @param length : The max length of the input
 */
extern void ui_input(char* title, char* buffer, uint8_t length);

/**
 * @brief Interrupt the UI to scroll a message and slowly play an animation
 * @param message : Message to scroll
 * @param raw_file : file to play
 * @param duration : Length of time in seconds to interrupt
 */
extern void ui_interrupt(char *message, char *raw_file, uint32_t duration);

extern void ui_menu(ui_menu_t* p_menu);

/**
 * @brief Main menu
 */
extern void ui_menu_main(void *parameters);

/**
 * @brief Tell main menu to regenerate itself
 */
extern void ui_menu_main_regenerate();

/**
 * @brief Present a list to the user to pick from
 * @param title : title to present to user
 * @param items : array of strings to pick from
 * @param count : number of strings in the array
 * @return index of the selected item
 */
extern uint8_t ui_pick_list(char* title, char** items, uint8_t count);

/**
 * @brief Allow user to pick a name or enter their own. Save to badge state
 */
extern void ui_pick_name();

/**
 * @brief Generate a modal popup to indicate something to the user
 *
 * @param text : Text to display
 * @param fg : Foreground color
 * @param bg: Background color
 */
extern void ui_popup(char* text, color_565_t fg, color_565_t bg);

/**
 * @brief Generate an error modal popup to indicate something to the user
 * @param text : Text to display
 */
extern void ui_popup_error(char* text);

/**
 * @brief Generate an info modal popup to indicate something to the user
 *
 * @param text : Text to display
 */
extern void ui_popup_info(char* text);

/**
 * @brief Display shouts screen for the badge
 */
extern void ui_shouts();

/**
 * @brief Draw a text box supporting scrolling and word wrap
 * @return if truncated
 */
extern bool ui_textbox(int16_t x,
                       int16_t y,
                       uint16_t w,
                       uint16_t h,
                       int16_t scroll_y,
                       char* text,
                       color_565_t fg,
                       color_565_t bg);

/**
 * @brief Show a clock full screen
 */
extern void ui_time();