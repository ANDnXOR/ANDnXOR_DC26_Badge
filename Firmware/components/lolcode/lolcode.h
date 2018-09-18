/*
 * lolcode.h
 *
 *  Created on: Aug 28, 2017
 *      Author: zapp
 */

#ifndef COMPONENTS_LOLCODE_LOLCODE_H_
#define COMPONENTS_LOLCODE_LOLCODE_H_

#include <stdbool.h>

#include "interpreter.h"

#define LULZ_API_LEVEL			2600
#define LULZ_MAX_FILE_SIZE		32000
#define LULZ_GLOBAL_MAX_SIZE	8
typedef struct {
	char key[LULZ_GLOBAL_MAX_SIZE + 1];
	char value[LULZ_GLOBAL_MAX_SIZE + 1];
	void *p_next;
} lulz_global_t;

extern bool lolcode_execute(const char *file_name, bool show_loading);
extern void task_lulzcode(void *pvParameters);

/**
 * Get a global value.
 *
 * @param 	key		Key of the value to retrieve
 * @return 			The value at key unless not found where NULL is returned
 */
extern char *lulz_global_get(char *key);

/**
 * Print the current globals to the ESP32 log
 */
extern void lulz_global_print();

/**
 * Set a global value.
 * Note: The key and value are limited to LULZ_GLOBAL_MAX_SIZE characters each and will be automatically null terminated.
 * Note: Global storage is volatile and will be reset when power is lost
 *
 * @param 	key		Key to store the value at
 * @param 	value	Value to store in globals
 */
extern void lulz_global_set(char *key, char *value);

#endif /* COMPONENTS_LOLCODE_LOLCODE_H_ */
