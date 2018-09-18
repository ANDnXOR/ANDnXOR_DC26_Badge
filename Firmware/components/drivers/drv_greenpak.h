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
#ifndef COMPONENTS_DRIVERS_DRV_GREENPAK_H_
#define COMPONENTS_DRIVERS_DRV_GREENPAK_H_

extern void drv_greenpak_flash(char* path);
extern void drv_greenpak_init();
extern void drv_greenpak_button_poll();
extern uint8_t drv_greenpak_button_state_read();

/**
 * @brief Read state of the combo LUT output
 */
extern uint8_t drv_greenpak_combo_read();

/**
 * @brief Read the bagel pin.
 */
extern uint8_t drv_greenpak_read_bagel_pin();

/**
 * @brief Read the toast pin.
 */
extern uint8_t drv_greenpak_read_toast_pin();

#endif /* COMPONENTS_DRIVERS_DRV_GREENPAK_H_ */
