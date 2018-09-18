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
#ifndef COMPONENTS_POST_H_
#define COMPONENTS_POST_H_

#define POST_STATE_NVS_KEY "post"

typedef struct {
  bool accelerometer_ack;
  bool button_up;
  bool button_down;
  bool button_left;
  bool button_right;
  bool button_a;
  bool button_b;
  bool button_c;
  bool greenpak_ack;
  bool greenpak_read_memory;
  bool led_driver_ack;
  bool sd_card_peripheral;
  bool sd_card_read;
  bool sd_card_write;
  bool usb_voltage;
} post_state_t;

extern void post_dump();
extern post_state_t* post_state_get();

/**
 * @brief Display POST status on screen
 */
extern void post_screen();

#endif /* COMPONENTS_POST_H_ */
