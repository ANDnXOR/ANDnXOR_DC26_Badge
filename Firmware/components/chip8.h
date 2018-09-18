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
#ifndef CHIP8_H_
#define CHIP8_H_

#include "system.h"

#define PC_START 0x200
#define CHIP8_NAME_MAX_LENGTH	10

typedef struct {
	char name[CHIP8_NAME_MAX_LENGTH];
	uint16_t len;				//Length (in bytes) of game data
	uint8_t key_mappings[16];	//Mapping of 16 physical chip-8 buttons to MAN BEAR PIG button masks
	uint8_t *game_data;			//Pointer to raw game data
	uint16_t hz;				//Game speed in hertz
	char *note;					//Pointer to game note
} chip8_game_t;

typedef struct {
	uint16_t opcode;
	uint8_t memory[4096];
	uint8_t V[16];
	uint16_t I;
	uint16_t pc;
	uint8_t gfx[128 * 64];
	uint8_t delay_timer;
	uint8_t sound_timer;
	uint16_t stack[16];
	uint8_t sp;
//	uint8_t key[16];
	uint8_t draw_flag;
	uint8_t extended_graphics_flag;
	volatile uint8_t exit_flag;
	chip8_game_t *p_game;
	uint8_t rpl[8]; 			//RPL storage to mimic HP48 "storage"s
} chip8_t;

extern void chip8_menu();
extern void chip8_run(chip8_game_t *p_game);
extern void chip8_run_file(char *filename);

extern void blinky_callback(void *data);
extern void floppy_callback(void *data);
extern void piper_callback(void *data);
extern void rush_hour_callback(void *data);
extern void tetris_callback(void *data);
extern void uboat_callback(void *data);
extern void wipeoff_callback(void *data);

#endif /* CHIP8_H_ */
