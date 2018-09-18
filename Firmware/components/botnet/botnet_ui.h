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
#ifndef COMPONENTS_BOTNET_BOTNET_UI_H_
#define COMPONENTS_BOTNET_BOTNET_UI_H_

__attribute__((unused)) static const char *botnet_payload_bg[] = {
    "",                                     // Main screen
    "/sdcard/gfx/botnet-bg.raw",            // Botnet Help
    "/sdcard/gfx/botnet-join.raw",          // Join Botnet
    "/sdcard/gfx/botnet-clean.raw",         // Innoculate
    "/sdcard/gfx/botnet-rick.raw",          // Bling
    "/sdcard/gfx/botnet-console.raw",       // Console poison
    "/sdcard/gfx/botnet-invert-colors.raw", // Invert color
    "/sdcard/gfx/botnet-invert-colors.raw", // Use same background for reverse
                                            // screen payload
    "/sdcard/gfx/botnet-buttons.raw",       //
    "/sdcard/gfx/botnet-quit.raw",          //
    "/sdcard/gfx/botnet-no-bling.raw",      //
    "/sdcard/gfx/botnet-trash.raw",         //
    "/sdcard/gfx/botnet-wifi.raw",          //
    "/sdcard/gfx/botnet-wifi.raw",          //
    "/sdcard/gfx/botnet-anonymous.raw",     // Badgelife Mimic
    "/sdcard/gfx/botnet-reset.raw",         // Master reset
    ""                                      // Count, not used
};

typedef enum
{
  botnet_screen_attack,
  botnet_screen_help,
  botnet_screen_join,
  botnet_screen_clean,
  botnet_screen_bling,
  botnet_screen_history_poison,
  botnet_screen_quit,
  botnet_screen_invert_colors,
  botnet_screen_reverse_screen,
  botnet_screen_buttons,
  botnet_screen_no_bling,
  botnet_screen_unlocks_delete,
#ifdef CONFIG_BADGE_TYPE_MASTER
  botnet_screen_ap_rick_roll,
  botnet_screen_ap_d4rkm4tter,
  botnet_screen_badgelife_mimic,
  botnet_screen_reset,
#endif
  botnet_screen_count
} botnet_screen_t;

extern void botnet_ui_main();

#endif /* COMPONENTS_BOTNET_BOTNET_UI_H_ */
