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
 *   @andnxor
 *   @zappbrandnxor
 *   @hyr0n1
 *   @exc3ls1or
 *   @lacosteaef
 *   @bitstr3m
 *****************************************************************************/
#pragma once
#include "system.h"

#ifndef CONSOLE_H_
#define CONSOLE_H_

#define PROMPT "AND!XOR> "
#define CONSOLE_HISTORY_LEN 100
#if CONFIG_STORE_HISTORY
#define CONSOLE_HISTORY_PATH "/sdcard/history.txt"
#endif

#define CONSOLE_BINARY_HIDDEN "/sdcard/ski/dirt.raw"
#define CONSOLE_BINARY_REVEAL "/sdcard/TacOS_Corp_login"

//Puzzle Specific Constants
#define CONSOLE_PHONE_NUM		"510-858-1337"
#define CONSOLE_WIFI_SSID 	"DRONELIFE"
#define CONSOLE_WIFI_PASS 	"" //There is NO Password, its an open AP...at DEF CON *lulz*
#define CONSOLE_WIFI_SITE 	"http://lulz.ninja/deecee26/1qazbgg5.txt"

// Register system functions
void register_system();
void console_gfx_welcome_msg();
void console_gfx_area_banner();
void console_gfx_area_eggz();
void console_gfx_game_over();
void console_gfx_game_over_alcohol_poisoning();
void console_gfx_game_win();

extern void initialize_console_data();

/**
 * @brief Pause the console task
 */
extern void console_task_pause();

/**
 * @brief Resume the console task
 */
extern void console_task_resume();
extern void console_task_start();

//Console Puzzle Data Structures
typedef struct{
	char name [26];
	float volume;
	float ABV;
}b00z3_t;

typedef struct{ 
	//Each item is specific to a location and comes with a beer
    char name[32];
    b00z3_t beer;
    bool haz;
}item_t;

typedef struct{
	uint8_t location;
	int WEIGHT;
	bool WEIGHT_CHANGE;
	float BAC;
	uint32_t FIRST_DRINK;
	char GENDER;
	bool GENDER_CHANGE;
	bool LOCATION_PUZZLE_SOLVED[5];
	item_t l00t[5];
	bool GAME_WON;
	bool HOME_AREA_COMPLETE;
	char NORTH_AREA_PIN[4];
	int NORTH_AREA_PIN_ATTEMPTS;
	int SOUTH_AREA_PW_FAILS;
	int SOUTH_AREA_PW_CORRECT;
	bool TREVOR;
}game_data_t;
#endif