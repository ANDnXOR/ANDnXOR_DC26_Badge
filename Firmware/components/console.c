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

#include "system.h"


const static char* TAG = "MRMEESEEKS::Console";
static TaskHandle_t m_console_task_handle = NULL;

static void initialize_console() {
  /* Disable buffering on stdin and stdout */
  setvbuf(stdin, NULL, _IONBF, 0);
  setvbuf(stdout, NULL, _IONBF, 0);

  /* Minicom, screen, idf_monitor send CR when ENTER key is pressed */
  esp_vfs_dev_uart_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
  /* Move the caret to the beginning of the next line on '\n' */
  esp_vfs_dev_uart_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

  /* Tell VFS to use UART driver */
  esp_vfs_dev_uart_use_driver(CONFIG_CONSOLE_UART_NUM);

  /* Initialize the console */
  esp_console_config_t console_config = {
    .max_cmdline_args = 8,
    .max_cmdline_length = 256,
#if CONFIG_LOG_COLORS
    .hint_color = atoi(LOG_COLOR_CYAN)
#endif
  };
  ESP_ERROR_CHECK(esp_console_init(&console_config));

  /* Configure linenoise line completion library */
  /* Enable multiline editing. If not set, long commands will scroll within
   * single line.
   */
  linenoiseSetMultiLine(1);

  /* Tell linenoise where to get command completions and hints */
  linenoiseSetCompletionCallback(&esp_console_get_completion);
  linenoiseSetHintsCallback((linenoiseHintsCallback*)&esp_console_get_hint);

  /* Set command history size */
  linenoiseHistorySetMaxLen(CONSOLE_HISTORY_LEN);

#ifdef CONFIG_STORE_HISTORY
  /* Load command history from filesystem */
  linenoiseHistoryLoad(CONSOLE_HISTORY_PATH);
#endif

  ESP_LOGI(TAG, "Console started");
}

static void __console_task(void* parameters) {
  /*
         #if CONFIG_STORE_HISTORY
         initialize_filesystem();
         #endif
         */

  initialize_console();

  /* Register commands */
  esp_console_register_help_command();
  register_system();

  /* Prompt to be printed before each line. This can be customized, made
   * dynamic, etc. */
  const char* prompt = LOG_COLOR_I PROMPT LOG_RESET_COLOR;

  // Intro Message
  printf(
      "\n"
      "Welcome to the AND!XOR DC26 Badge Challenge: B.E.N.D.E.R.\n\n"
      "The Badge-Enabled Non-Directive Enigma Routine\n\n"
      "You awake dead center in the middle of the Nevada desert.\n"
      "The heat is blistering, you have no water, or idea how you got there.\n"
      "For the time being it appears that this is your new home.\n"
      "By default you are a Non-Binary 400lb Hacker.\n"
      "You should first setup your player profile using the gender and weight "
      "commands.\n"
      "Type 'help' to get the list of commands.\n\n");

  /* Figure out if the terminal supports escape sequences */
  int probe_status = linenoiseProbe();
  if (probe_status) { /* zero indicates success */
    printf(
        "\n"
        "Your terminal application does not support escape sequences.\n"
        "Line editing and history features are disabled.\n"
        "Use PuTTY instead!\n");
    linenoiseSetDumbMode(1);

#if CONFIG_LOG_COLORS
    /* Since the terminal doesn't support escape sequences, don't use color
     * codes in the prompt. */
    prompt = PROMPT;
#endif  // CONFIG_LOG_COLORS
  }

  // Main loop runs forever
  while (true) {
    // Get a line using linenoise. The line is returned when ENTER is pressed.
    char* line = linenoise(prompt);
    if (line == NULL) { /* Ignore empty lines */
      continue;
    }
    /* Add the command to the history */
    linenoiseHistoryAdd(line);

#ifdef CONFIG_STORE_HISTORY
    /* Save command history to filesystem */
    linenoiseHistorySave(CONSOLE_HISTORY_PATH);
#endif

    /* Try to run the command */
    int ret;
    esp_err_t err = esp_console_run(line, &ret);
    if (err == ESP_ERR_NOT_FOUND) {
      printf("That doesn't make sense. You must be drunk...\n\n");
    } else if (err == ESP_OK && ret != ESP_OK) {
      printf("Command returned non-zero error code: 0x%x\n\n", ret);
    } else if (err != ESP_OK) {
      printf("Internal error: 0x%x\n\n", err);
    }
    /* linenoise allocates line buffer on the heap, so need to free it */
    linenoiseFree(line);
  }

  // Just in case in breaks free of the while()
  vTaskDelete(NULL);
}

/**
 * @brief Pause the console task
 */
void console_task_pause() {
  if (m_console_task_handle != NULL) {
    ESP_LOGD(TAG, "Pausing console");
    vTaskSuspend(m_console_task_handle);
  }
}

/**
 * @brief Resume the console task
 */
void console_task_resume() {
  if (m_console_task_handle != NULL) {
    ESP_LOGD(TAG, "Resuming Console");
    vTaskResume(m_console_task_handle);
  }
}

void console_task_start() {
  ESP_LOGI(TAG, "Starting Console");
  m_console_task_handle = xTaskCreate(
      __console_task, "Console", 8000, NULL, TASK_PRIORITY_CONSOLE, NULL);
}
