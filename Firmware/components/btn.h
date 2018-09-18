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
#ifndef COMPONENTS_BTN_H_
#define COMPONENTS_BTN_H_

#define GPIO_GREENPAK_ISR GPIO_NUM_34
#define GPIO_SEL_GREENPAK_ISR GPIO_SEL_34
#define BUTTON_GREENPAK_ISR_MASK 0b10000000

#define BUTTON_MASK_UP 0b10000000
#define BUTTON_MASK_DOWN 0b00000100
#define BUTTON_MASK_LEFT 0b00001000
#define BUTTON_MASK_RIGHT 0b00000010
#define BUTTON_MASK_A 0b00100000
#define BUTTON_MASK_B 0b01000000
#define BUTTON_MASK_C 0b00010000

#define GPIO_BTN_MASK (GPIO_SEL_GREENPAK_ISR)

/**
 * @brief Enable or disable user input
 * @param user_input : set to true to allow user input
 */
extern void btn_allow_user_input(bool user_input);

/**
 * @brief Clear the current button state
 */
extern void btn_clear();

/**
 * Initialize the button high level API
 */
extern void btn_init();
extern uint8_t btn_state();

extern bool btn_down();
extern bool btn_up();
extern bool btn_left();
extern bool btn_right();
extern bool btn_a();
extern bool btn_b();
extern bool btn_c();

/**
 * @brief Set whether or not to randomize buttons
 */
extern void btn_randomize(bool randomize);

/**
 * Block forever until a button press
 */
extern uint8_t btn_wait();

/**
 * Block until button press with a max wait time
 */
extern uint8_t btn_wait_max(uint32_t max_wait_ms);

#endif /* COMPONENTS_BTN_H_ */
