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
static void register_walk();
static void register_N();
static void register_S();
static void register_E();
static void register_W();
static void register_look();
static void register_l00t();
static void register_steal();
static void register_drink();
static void register_hack();
static void register_status();
static void register_gender();
static void register_weight();
static void system_register_memory();
static void system_register_name();
static void system_register_peers();
static void system_register_ps();

// Players are going to have to google the beers to figure out the ABV
static float BEER_0_ABV = 0.071;  // MadeWest IPA
static float BEER_1_ABV = 0.050;  // Mother Earth CaliCreamin
static float BEER_2_ABV = 0.082;  // Elysian Spacedust
static float BEER_3_ABV = 0.065;  // Dagger Falls Sockeye IPA
static float BEER_4_ABV = 0.087;  // Modern Times Tetra City Double IPA
static char* HOME_AREA_ITEM = "BOTTLE_OPENER";               // ITEM 0
static char* HOME_AREA_BEER = "MADEWEST_IPA";                // BEER 0
static char* NORTH_AREA_ITEM = "EMMC_ADAPTER";               // ITEM 1
static char* NORTH_AREA_BEER = "MOTHER_EARTH_CALI_CREAMIN";  // BEER 1
static char* SOUTH_AREA_ITEM = "USB_CABLE";                  // ITEM 2
static char* SOUTH_AREA_BEER = "ELYSIAN_SPACE_DUST";         // BEER 2
static char* EAST_AREA_ITEM = "WIFI_CACTUS";                 // ITEM 3
static char* EAST_AREA_BEER = "DAGGER_FALLS_SOCKEYE_IPA";    // BEER 3
static char* WEST_AREA_ITEM = "BURNER_PHONE";                // ITEM 4
static char* WEST_AREA_BEER = "MODERN_TIMES_TETRA_CITY";     // BEER 4
static char* WEST_AREA_PASSWORD = "1337KopterSauce";
const static char* TAG = "MRMEESEEKS::CONSOLE_WiFi";

void initialize_console_data() {
  // Point to stateful data storage in NVS
  game_data_t* p_state = state_console_get();

  // Build l00t and beer data with STRCPY the M0ST S3CUR3 EVER!
  strcpy(p_state->l00t[0].name, HOME_AREA_ITEM);
  strcpy(p_state->l00t[0].beer.name, HOME_AREA_BEER);
  strcpy(p_state->l00t[1].name, NORTH_AREA_ITEM);
  strcpy(p_state->l00t[1].beer.name, NORTH_AREA_BEER);
  strcpy(p_state->l00t[2].name, SOUTH_AREA_ITEM);
  strcpy(p_state->l00t[2].beer.name, SOUTH_AREA_BEER);
  strcpy(p_state->l00t[3].name, EAST_AREA_ITEM);
  strcpy(p_state->l00t[3].beer.name, EAST_AREA_BEER);
  strcpy(p_state->l00t[4].name, WEST_AREA_ITEM);
  strcpy(p_state->l00t[4].beer.name, WEST_AREA_BEER);
  p_state->l00t[0].beer.ABV = BEER_0_ABV;
  p_state->l00t[1].beer.ABV = BEER_1_ABV;
  p_state->l00t[2].beer.ABV = BEER_2_ABV;
  p_state->l00t[3].beer.ABV = BEER_3_ABV;
  p_state->l00t[4].beer.ABV = BEER_4_ABV;

  for (int i = 0; i <= 4; i++) {
    p_state->l00t[i].haz = false;
    p_state->l00t[i].beer.volume = 22;
    p_state->LOCATION_PUZZLE_SOLVED[i] = false;
  }

  p_state->LOCATION_PUZZLE_SOLVED[0] = true; // Makes it so the first item can be stolen
  p_state->BAC = 0.0;     // Start sober
  p_state->FIRST_DRINK = 0; //Set to epoch start, means you haven't drank yet
  p_state->location = 0;  // Start at home

  // Generate North Area Hardware Hacking Puzzle Pin
  for (int i = 0; i < 4; i++) {
    p_state->NORTH_AREA_PIN[i] = '0' + (esp_random() % 4) + 1;
  }
  p_state->NORTH_AREA_PIN_ATTEMPTS = 0;

  // Setup South Area Brute Force and Win Tracker
  p_state->SOUTH_AREA_PW_FAILS = 0;
  p_state->SOUTH_AREA_PW_CORRECT = 0;

  // Set Default Gender (Non Binary) and Weight (400lb Hacker)
  p_state->GENDER = 'N';
  p_state->GENDER_CHANGE = false;
  p_state->WEIGHT = 400;
  p_state->WEIGHT_CHANGE = false;

  // You dont have Trevor...yet...
  p_state->TREVOR = false;

  // You haven't beaten the game yet
  p_state->HOME_AREA_COMPLETE = false;
  p_state->GAME_WON = false;
}

void welcome_message() {
  printf("Press ENTER key to begin...\n\n");
  getchar();
  console_gfx_welcome_msg();
}

void register_system() {
  // This is an initializer method
  register_drink();
  register_hack();
  register_steal();
  register_l00t();
  register_look();
  register_status();
  register_walk();
  register_N();
  register_S();
  register_E();
  register_W();
  register_gender();
  register_weight();
  system_register_memory();
  system_register_name();
  system_register_peers();
  system_register_ps();
  welcome_message();
}

int download_callback(request_t* req, char* data, int len) {
  // West Area WiFi Puzzle
  req_list_t* found = req->response->header;
  while (found->next != NULL) {
    found = found->next;
    ESP_LOGI(TAG, "Response header %s:%s", (char*)found->key, (char*)found->value);
  }
  // or
  found = req_list_get_key(req->response->header, "Content-Length");
  if (found) {
    ESP_LOGI(TAG, "Get header %s:%s", (char*)found->key, (char*)found->value);
  }
  ESP_LOGI(TAG, "%s", data);
  return 0;
}

void puzzle_get_request() {
  // West Area WiFi Puzzle
  request_t* req;
  ESP_LOGI(TAG, "Attempting to connect to to website..., freemem=%d", esp_get_free_heap_size());
  req = req_new(CONSOLE_WIFI_SITE);
  req_setopt(req, REQ_SET_METHOD, "GET");
  req_setopt(req, REQ_FUNC_DOWNLOAD_CB, download_callback);
  req_perform(req);
  req_clean(req);
}

float get_BAC() {
  game_data_t* p_state = state_console_get();
  double time_h = 0;
  int difference = time_manager_now_sec() - p_state->FIRST_DRINK;
  double final_BAC = p_state->BAC;

  if(p_state->FIRST_DRINK!=0){//Convert seconds to hours
    time_h = (double)difference/3600;
  }
  
  if (difference > 0){
    if(final_BAC - (time_h * 0.015) > 0)
      return final_BAC - (time_h * 0.015); //Normal case for BAC calculation
    else
      return 0; //This prevents your BAC from going into the negatives
  }
  else
    return final_BAC; //odd case where time loss causes you to have drank in the future
}

void set_BAC(float oz, float abv) {
  game_data_t* p_state = state_console_get();

  float GENDER_CONSTANT;

  if (p_state->GENDER == 'M')
    GENDER_CONSTANT = 0.73;
  else if (p_state->GENDER == 'F')
    GENDER_CONSTANT = 0.66;
  else
    GENDER_CONSTANT =
        0.695;  // Non-Binary Gender Determination BAC constant, am I the only one who thinks about these things?

  //Initialize Drinking Variables incase you sobered up or its your first time
  if(get_BAC() == 0){
    p_state->BAC = 0; //get_BAC being 0 doesnt imply BAC is actually 0, since get_BAC incorporates time degredation
    p_state->FIRST_DRINK = time_manager_now_sec(); //reset the drinking time
  }

  //Set Initial Drinking Timestamp
  //if(p_state->FIRST_DRINK == 0)
  //  p_state->FIRST_DRINK = time_manager_now_sec();

  // Update BAC
  p_state->BAC += (oz * abv * 5.14) / (p_state->WEIGHT * GENDER_CONSTANT);
}

int copy_file(char* old_filename, char* new_filename) {
  // This is used in the south area puzzle to reveal a binary hidden elsewhere on the SD Card
  int a;

  FILE* ptr_old = fopen(old_filename, "r");
  if (!ptr_old) {
    ESP_LOGE(TAG, "Could not open '%s' for read. It doesn't exist. errno=%d", old_filename, errno);
    return -1;
  }

  FILE* ptr_new = fopen(new_filename, "w");
  if (!ptr_new) {
    ESP_LOGE(TAG, "Could not open '%s' for write. errno=%d", new_filename, errno);
    return -1;
  }

  while (1) {
    a = fgetc(ptr_old);

    if (!feof(ptr_old))
      fputc(a, ptr_new);
    else
      break;
  }

  fclose(ptr_new);
  fclose(ptr_old);
  return 0;
}

static int weight(int argc, char** argv) {
  game_data_t* p_state = state_console_get();
  // Change the players weight
  bool error_flag = false;

  if (p_state->WEIGHT_CHANGE == true) {
    printf("  You already changed your weight. Deal with it or restart the game.\n\n");
  } else if ((argc == 2) && (strtof(argv[1], NULL) >= 100) && (strtof(argv[1], NULL) <= 400)) {
      p_state->WEIGHT = strtof(argv[1], NULL);
      printf("  Weight change successful!\n\n");
      p_state->WEIGHT_CHANGE = true;
  } else
    error_flag = true;

  if (error_flag) {
    printf("  That doesn't make sense...you must be drunk \n\n");
  }
  return 0;
}

static void register_weight() {
  const esp_console_cmd_t cmd = {
      .command = "weight",
      .help = "Change your weight...you can only do this ONCE!",
      .hint = "[100lb <= w <= 400lb]",
      .func = &weight};
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int gender(int argc, char** argv) {
  game_data_t* p_state = state_console_get();
  // Change the players gender
  bool error_flag = false;

  if (p_state->GENDER_CHANGE == true) {
    printf(
        "  You already changed your gender. Deal with it or restart the game.\n\n");
  } else if (argc == 2) {
    if ((strcmp(argv[1], "M") == 0) || (strcmp(argv[1], "m") == 0)) {
      p_state->GENDER = 'M';
      p_state->GENDER_CHANGE = true;
      printf("  Sex change successful: Man-Bot!\n\n");
    } else if ((strcmp(argv[1], "F") == 0) || (strcmp(argv[1], "f") == 0)) {
      p_state->GENDER = 'F';
      p_state->GENDER_CHANGE = true;
      printf("  Sex change successful: Fem-Bot!\n\n");
    } else if ((strcmp(argv[1], "N") == 0) || (strcmp(argv[1], "n") == 0)) {
      p_state->GENDER = 'N';
      p_state->GENDER_CHANGE = true;
      printf("  Sex change successful: NonBinary-Bot!\n\n");
    } else
      error_flag = true;
  } else
    error_flag = true;

  if (error_flag) {
    printf("  That doesn't make sense...you must be gender drunk \n\n");
  }
  return 0;
}

static void register_gender() {
  const esp_console_cmd_t cmd = {
      .command = "gender",
      .help = "Gender Bender to a Fem-bot (F), Man-bot (M), or NonBinary-bot (N) ...you can only do this ONCE!",
      .hint = "[determination F, M, or N]",
      .func = &gender};
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int status(int argc, char** argv) {
  game_data_t* p_state = state_console_get();

  char name[STATE_NAME_LENGTH + 1];
  state_name_get(name);
  printf("  Name: %s \n", name);

  switch (p_state->location) {
    case 0:
      printf("  Location: Home \n");
      break;
    case 1:
      printf("  Location: North \n");
      break;
    case 2:
      printf("  Location: South \n");
      break;
    case 3:
      printf("  Location: East \n");
      break;
    case 4:
      printf("  Location: West \n");
      break;
  }
  printf("  Gender: %c \n", p_state->GENDER);
  printf("  Weight: %i lbs \n", p_state->WEIGHT);
  printf("  Blood Alcohol Content: %.4f\n", get_BAC());

  //How long since your first drink?
  if (get_BAC() == 0)
    printf("  First Boozin UTC: U IZ SOBR\n");
  else
    printf("  First Boozin UTC: %s", asctime(gmtime((time_t *)&p_state->FIRST_DRINK)));
  
  uint32_t cur_time = time_manager_now_sec();
  printf("  Current Time UTC: %s \n", asctime(gmtime((time_t *)&cur_time)));
  return 0;
}

static void register_status() {
  const esp_console_cmd_t cmd = {
      .command = "status",
      .help = "Show the status of the player",
      .hint = NULL,
      .func = &status,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static void display_morse(char* m_code) {
  // This is only meant for numbers > 10 (cuz I'm lazy), that means all strings are 12 characters long once in morse code
  //Initial delay
  DELAY(300);

  // Process each character
  for (int i = 0; i < 11; i++) {
    switch (m_code[i]) {
      case '.': {
        led_set_all(0, 255, 0);  // GREEN
        led_show();
        DELAY(300);
        led_clear();
        DELAY(300);
      } break;

      case '-': {
        led_set_all(255, 0, 0);  // RED
        led_show();
        DELAY(1200);
        led_clear();
        DELAY(300);
      } break;

      case ' ': {
        led_clear();
        led_show();
        DELAY(600);
      } break;
    }
  }
  DELAY(1337);  // Delay before processing next number
}

static struct {
  struct arg_str* b00z3;
  struct arg_str* beer_oz;
  struct arg_end* end;
} drink_args;

static int drink(int argc, char** argv) {
  game_data_t* p_state = state_console_get();

  if (p_state->GENDER_CHANGE == false) {
    printf("  You haven't set your gender yet...\n\n");
    return 0;
  }

  if (p_state->WEIGHT_CHANGE == false) {
    printf("  You haven't set your weight yet...\n\n");
    return 0;
  }

  if (p_state->l00t[0].haz == true) {
    bool error_flag = false;

    if (argc == 3) {
      for (int i = 0; i <= 4;
           i++) {  // You can only drink a beer if you have it, there is enough volume, the volume is positive, and the beer actually exists
        if ((strcmp(argv[1], p_state->l00t[i].beer.name) == 0) && (p_state->l00t[i].beer.volume >= strtof(argv[2], NULL)) &&
            (strtof(argv[2], NULL) > 0) && (p_state->l00t[i].haz)) {

          // Calculate your new BAC
          set_BAC(strtof(argv[2], NULL), p_state->l00t[i].beer.ABV);

          // Subtract b00z3 you drank from the bottle
          p_state->l00t[i].beer.volume -= strtof(argv[2], NULL);

          // Display Results
          printf("  You down some delicious craft b33r \n");
          printf("  Your BAC is now %.4f \n\n", get_BAC());
          error_flag = false;
          break;
        } else if ((strcmp(argv[1], "piss") == 0) ||
                   (strcmp(argv[1], "budweiser") == 0) ||
                   (strcmp(argv[1], "coors") == 0)) {
          printf("  Why the hell would you do that!?! Gross!!! \n\n");
          break;
        } else
          error_flag = true;
      }
    } else
      error_flag = true;

    if (error_flag) {
      printf("  That doesn't make sense...you can't drink that \n\n");
    }

    // Kill player if they drink too much b00z3
    if (get_BAC() >= 0.3) {
      printf("  You drank too much b00z3 and die of dysentery...err...alcohol poisoning\n\n");
      printf("  GAME OVER!\n\n");
      console_gfx_game_over_alcohol_poisoning();
      welcome_message();
      initialize_console_data();
    }
  } else {
    printf("  You can't open the bottle without a bottle opener...\n\n");
  }
  return 0;
}

static void register_drink() {
  drink_args.b00z3 = arg_str0(NULL, NULL, "b00z3", NULL);
  drink_args.beer_oz = arg_str1(NULL, NULL, "[volume in oz]", NULL);
  drink_args.end = arg_end(2);

  const esp_console_cmd_t cmd = {.command = "drink",
    .help = "Drink all the b00z3...",
    .hint = NULL,
    .func = &drink,
    .argtable = &drink_args};
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static struct {
  struct arg_str* thingz;
  struct arg_end* end;
} steal_args;

static int steal(int argc, char** argv) {
  game_data_t* p_state = state_console_get();
  bool error_flag = false;

  if (argc == 2) {
    if ((strcmp(argv[1], p_state->l00t[p_state->location].name) == 0) &&
        (p_state->l00t[p_state->location].haz == false) &&
        (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == true)) {
      p_state->l00t[p_state->location].haz = true;
      printf("  You stole a %s \n", p_state->l00t[p_state->location].name);
      printf("  You also found a beer! A delicious bomber of %s \n\n", p_state->l00t[p_state->location].beer.name);
    } else if ((strcmp(argv[1], p_state->l00t[p_state->location].name) == 0) &&
               (p_state->l00t[p_state->location].haz == true) &&
               (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == true)) {
      printf("  You already stole that...you must be drunk...\n\n");
    } else if ((strcmp(argv[1], "COCKROACH") == 0) && (p_state->TREVOR == false) && (p_state->location == 1)) {
      // Just for Trevor easter egg, I normally wouldn't do it this way, but crunch time...
      p_state->TREVOR = true;
      printf("  You put a dead smashed COCKROACH in your pocket...\n\n");
    } else
      error_flag = true;
  } else
    error_flag = true;

  if (error_flag) {
    printf("  That doesn't make sense...you can't steal that \n\n");
  }
  return 0;
}

static void register_steal() {
  steal_args.thingz = arg_str0(NULL, NULL, "thing", NULL);
  steal_args.end = arg_end(1);

  const esp_console_cmd_t cmd = {
      .command = "steal",
      .help = "Steal all the thingz you find as l00t",
      .hint = NULL,
      .func = &steal,
      .argtable = &steal_args};
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int walk(int argc, char** argv) {
  game_data_t* p_state = state_console_get();
  // Walk command moves the user in a direction bound by the trail paths
  bool error_flag = false;

  if (p_state->HOME_AREA_COMPLETE == false) {
    if (argc == 2) {
      // Location Reference: Home = 0, North = 1, South = 2, East = 3, West = 4
      if (strcmp(argv[1], "HOME") == 0) {
        if (p_state->location != 0) {
          p_state->location = 0;
          console_gfx_area_banner(p_state->location);
        } else
          printf("  You are already at good old 127.0.0.1\n\n");
      } else if (strcmp(argv[1], "NORTH") == 0) {
        // You can only leave home(0) to go north(1), or leave south(2) to go home(0)
        if (p_state->location == 0) {
          p_state->location = 1;
          console_gfx_area_banner(p_state->location);
        } else if (p_state->location == 2) {
          p_state->location = 0;
          console_gfx_area_banner(p_state->location);
        } else
          printf("You better stay on the trail...\n\n");
      } else if (strcmp(argv[1], "SOUTH") == 0) {
        // You can only leave home(0) to go south(2), or leave north(1) to go home(0)
        if (p_state->location == 0) {
          p_state->location = 2;
          console_gfx_area_banner(p_state->location);
        } else if (p_state->location == 1) {
          p_state->location = 0;
          console_gfx_area_banner(p_state->location);
        } else
          printf("  You better stay on the trail...\n\n");
      } else if (strcmp(argv[1], "EAST") == 0) {
        // You can only leave home(0) to go east(3), or leave west(4) to go home(0)
        if (p_state->location == 0) {
          p_state->location = 3;
          console_gfx_area_banner(p_state->location);
        } else if (p_state->location == 4) {
          p_state->location = 0;
          console_gfx_area_banner(p_state->location);
        } else
          printf("  You better stay on the trail...\n\n");
      } else if (strcmp(argv[1], "WEST") == 0) {
        // You can only leave home(0) to go west(4), or leave east(3) to go home(0)
        if (p_state->location == 0) {
          p_state->location = 4;
          console_gfx_area_banner(p_state->location);
        } else if (p_state->location == 3) {
          p_state->location = 0;
          console_gfx_area_banner(p_state->location);
        } else
          printf("  You better stay on the trail...\n\n");
      } else
        error_flag = true;
    } else
      error_flag = true;
  } else if ((p_state->HOME_AREA_COMPLETE == true) &&
             ((strcmp(argv[1], "NORTH") == 0) ||
              (strcmp(argv[1], "SOUTH") == 0) ||
              (strcmp(argv[1], "EAST") == 0) ||
              (strcmp(argv[1], "WEST") == 0))) {
    // You've beaten the final challenge and are in the last room, can't leave
    printf("  You can't walk anywhere, you're stuck in this room.\n\n");
  } else
    error_flag = true;

  if (error_flag) {
    printf("  That doesn't make sense... \n\n");
  }
  return 0;
}

static void register_walk() {
  const esp_console_cmd_t cmd = {
      .command = "walk",
      .help = "Walk to a new location: NORTH, SOUTH, EAST, WEST, or HOME",
      .hint = "[direction]",
      .func = &walk,
      //.argtable = &walk_args
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int N(int argc, char** argv) {
  int n = 6;
  char** array = (char**)util_heap_alloc_ext((n + 1) * sizeof(char*));
  array[0] = "walk";
  array[1] = "NORTH";
  array[3] = 0;

  walk(2, array);
  free(array);
  return 0;
}

static void register_N() {
  const esp_console_cmd_t cmd = {
      .command = "N",
      .help = "Relative directional shortcut for typing: walk NORTH",
      .func = &N,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int S(int argc, char** argv) {
  int n = 6;
  char** array = (char**)util_heap_alloc_ext((n + 1) * sizeof(char*));
  array[0] = "walk";
  array[1] = "SOUTH";
  array[3] = 0;

  walk(2, array);
  free(array);
  return 0;
}

static void register_S() {
  const esp_console_cmd_t cmd = {
      .command = "S",
      .help = "Relative directional shortcut for typing: walk SOUTH",
      .func = &S,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int E(int argc, char** argv) {
  int n = 6;
  char** array = (char**)util_heap_alloc_ext((n + 1) * sizeof(char*));
  array[0] = "walk";
  array[1] = "EAST";
  array[3] = 0;

  walk(2, array);
  free(array);
  return 0;
}

static void register_E() {
  const esp_console_cmd_t cmd = {
      .command = "E",
      .help = "Relative directional shortcut for typing: walk EAST",
      .func = &E,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int W(int argc, char** argv) {
  int n = 6;
  char** array = (char**)util_heap_alloc_ext((n + 1) * sizeof(char*));
  array[0] = "walk";
  array[1] = "WEST";
  array[3] = 0;

  walk(2, array);
  free(array);
  return 0;
}

static void register_W() {
  const esp_console_cmd_t cmd = {
      .command = "W",
      .help = "Relative directional shortcut for typing: walk WEST",
      .func = &W,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}
static struct {
  struct arg_str* thingz;
  struct arg_end* end;
} hack_args;

static int hack(int argc, char** argv) {
  game_data_t* p_state = state_console_get();

  bool error_flag = false;

  if ((argc == 4) && (strcmp(argv[2], "with") == 0)) {
    switch (p_state->location) {  // Check to see if THING1(arg1) is valid && THING2(arg3) is valid && THING2(arg3) is in your inventory
      case 0: {  // HOME AREA HACKS
        if ((strcmp(argv[1], "BREATHALYZER") == 0) && (strcmp(argv[3], "BREATH") == 0) && (p_state->HOME_AREA_COMPLETE == false)) {
          // check if all 5 beers were drank
          bool drank_all_the_b00z3 = true;
          for (int i = 0; i <= 4; i++) {
            if (p_state->l00t[i].beer.volume == 22.0)
              drank_all_the_b00z3 = false;
          }

          if (drank_all_the_b00z3) {
            // truncate the float to 4 decimal places & save as a string
            // its not 100% elegant but just as messy as any other method to
            // "chop" significant digits on a float
            char temp_BAC[7];
            snprintf(temp_BAC, 7, "%.4f", get_BAC());

            if (strcmp("0.1337", temp_BAC) == 0) {
              printf("  Your BAC is %.4f and the final challenge is solved!\n", get_BAC());
              printf("  The desert floor opens up from beneath and you fall into a chute. You think to yourself, OH CHUTE!!!\n");
              printf("  Your chute dumps you in to a multi-colored blinky lit room from dozens of BADGES.\n");
              printf("  One stands out, a HYPERCUBE_BADGE.\n");
              printf("  Aside from that, the floor is littered with used salsa cups and Taco Corp snack boats...\n");
              printf("  At the opposite end of the room is a door with a glowing EXIT_BUTTON.\n\n");
              // Set the completion flag
              p_state->HOME_AREA_COMPLETE = true;
            } else {
              printf("  Your BAC is %.4f \n", get_BAC());
              printf("  If its too low, keep drinking...\n");
              printf("  If its too high, wait it out...\n");
              printf("  Regardless, you suck at this game!\n\n");
            }
          } else {
            printf("  You have to drink from EVERY craft beer! Don't suck at this and follow instructions!\n\n");
          }
        } else if ((strcmp(argv[1], "HYPERCUBE_BADGE") == 0) &&
                   (strcmp(argv[3], "FINGER") == 0) &&
                   (p_state->HOME_AREA_COMPLETE == true) &&
                   (p_state->GAME_WON == false)) {
          // BAD GAME OVER
          printf("  You cant resist the wonderful glowing bling from this cube shaped badge.\n");
          printf("  Touching it makes everything seem so clear...and dark...the lights are now fading....\n");
          printf("  You were so close...why did you have to touch the cube?\n\n");
          console_gfx_game_over();
          welcome_message();
          initialize_console_data();
          break;
        } else if ((strcmp(argv[1], "EXIT_BUTTON") == 0) &&
                   (strcmp(argv[3], "FINGER") == 0) &&
                   (p_state->HOME_AREA_COMPLETE == true) &&
                   (p_state->GAME_WON == false)) {
          // GOOD GAME OVER
          console_gfx_game_win();
          printf("  You press the button and the door opens slowly, light begins to break through from the other side.\n");
          printf("  TO BE CONTINUED...\n\n");
          printf("  YOU HAVE COMPLETED THE AND!XOR DC26 BADGE CHALLENGE & EARNED 1337 XP FOR BOTNET!\n");
          printf("  CONTACT US ON TWITTER @ANDNXOR && EMAIL HYR0N@ANDNXOR.COM ASAP BEFORE THE CON IS OVER.\n");
          printf("  YOUR STATE WILL SAVE ON A TASK THAT RUNS EVERY 1 MIN, YOU MAY SAFELY DISCONNECT THE BADGE IN 1 MIN...\n\n");
          p_state->GAME_WON = true;
          state_unlock_set(state_unlock_get() | UNLOCK_CONSOLE_WIN);
          state_botnet_get()->experience += 1337;
        } else
          error_flag = true;
      } break;

      case 1: {  // NORTH AREA HACKS
        if ((strcmp(argv[1], "PIN_PAD") == 0) &&
            (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == false)) {
          char* attempt = argv[3];

          // Check Hack Attempt Counter, GAME OVER if >= 16 attempts
          if (p_state->NORTH_AREA_PIN_ATTEMPTS < 15)
            p_state->NORTH_AREA_PIN_ATTEMPTS++;
          else {
            printf("  You couldn't figure this out in under 16 attempts?...\n");
            printf("  The treadmill explodes sending Joe's shin's in a firework display of gibs.\n\n");
            console_gfx_game_over();
            welcome_message();
            initialize_console_data();
            break;
          }

          // Initial CLK Trigger
          led_set_all(255, 0, 0);  // RED
          led_show();
          DELAY(25);
          led_clear();

          // Compare the position of the guessed pin to the actual pin
          // The delay is so we can exaggerate a successful guess when viewing
          // CLK on a logic analyzer or o-scope
          for (int i = 0; i < 4; i++) {
            if (attempt[i] == p_state->NORTH_AREA_PIN[i])
              DELAY(25); //This is the delay between blinks which compounds to get longer the closer you are to solving
            else {
              printf("  NOPE!...%d attempt(s) left...\n\n",
                     16 - p_state->NORTH_AREA_PIN_ATTEMPTS);
              break;
            }
            if (i == 3) {
              p_state->LOCATION_PUZZLE_SOLVED[p_state->location] = true;
              printf(
                "  It appears the pin you entered on the PIN_PAD was correct.\n"
                "  Blinky status lights flicker on the junction box and the LED above the cable appears lit.\n"
                "  Joe glances over at you and gives a thumbs up.\n"
                "  He then grabs an item off his table and tosses it on the ground.\n"
                "  It slides just under the junction box. Joe says...\n\n"
                "  Take....EMMC_ADAPTER...no...use...to...me...while...running...\n\n");
            }
          }

          // Final CLK Trigger
          led_set_all(255, 0, 0);  // RED
          led_show();
          DELAY(25);
          led_clear();
        } else if ((strcmp(argv[1], "PHONE_RECYCLE_BIN") == 0) &&
                   (strcmp(argv[3], WEST_AREA_ITEM) == 0)) {
          printf("  You drop the BURNER_PHONE in the top of the PHONE_RECYCLE_BIN it bounces around inside the box.\n");
          printf("  It then just falls out a small hole in the bottom and lands at your shoes.\n");
          printf("  Looks like the electronics store can't do anything right with regards to cell phones...\n");
          printf("  You notice there's a complaint line on the side of the box: %s\n\n", CONSOLE_PHONE_NUM);
        } else
          error_flag = true;
      } break;

      case 2: {  // SOUTH AREA HACKS
        if ((strcmp(argv[1], "SALSA") == 0) &&
            (strcmp(argv[3], "FINGER") == 0)) {
          printf("  You begin sampling from the salsa bar. As you stick your finger in the tomatillo...\n");
          printf("  There's a weird squarish flat object at the bottom.\n");
          printf("  Instictively you go into YOLO-mode and pop it in your mouth to clean off the salsa.\n");
          printf("  Rolling the object around in your mouth, the static object almost tastes...binary...\n");
          printf("  Thats some existential cyberpunk shit right there. Freaked, you spit out what appears to be an SD card!\n");
          printf("  It bounces back in to the bowl of tomatillo salsa...but you feel like somehow...somewhere...\n");
          printf("  You got a copy of whatever was on that SD Card...\n\n");
          int result = copy_file(CONSOLE_BINARY_HIDDEN, CONSOLE_BINARY_REVEAL);
          ESP_LOGI(TAG, "Hidden File copy result: %d", result);
        } else if ((strcmp(argv[1], "TOUCHSCREEN") == 0) &&
                   (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] ==
                    false)) {
          if ((strcmp(argv[3], "555") == 0) &&
              (p_state->SOUTH_AREA_PW_CORRECT == 0)) {
            p_state->SOUTH_AREA_PW_CORRECT++;
            printf("  1ST PASSWORD ACCEPTED!\n");
            printf("  ENTER 2ND PASSWORD: \n\n");
          } else if ((strcmp(argv[3], "CorrectHorseBatteryStaple") == 0) &&
                     (p_state->SOUTH_AREA_PW_CORRECT == 1)) {
            p_state->SOUTH_AREA_PW_CORRECT++;
            printf("  2ND PASSWORD ACCEPTED!\n");
            printf("  ENTER 3RD PASSWORD: \n\n");
          } else if ((strcmp(argv[3], "BLU3PaNC@K3$") == 0) &&
                     (p_state->SOUTH_AREA_PW_CORRECT == 2)) {
            p_state->SOUTH_AREA_PW_CORRECT++;
            p_state->LOCATION_PUZZLE_SOLVED[p_state->location] = true;
            printf("  3RD PASSWORD ACCEPTED!\n");
            printf("  ACCESS GRANTED To Taco Corp Core Operating System: TacOS.\n\n");
            printf("  You are logged into the TOUCHSCREEN terminal with a familiar $ prompt.\n");
            printf("  The thought crosses your mind \"It's a LINUX system, I know this...\"\n");
            printf("  With haste you type $service generator start\n");
            printf("  The taco truck generator comes to life...\n\n");
          } else {
            // Check Hack Attempt Counter, GAME OVER if >= 16 attempts
            if (p_state->SOUTH_AREA_PW_FAILS < 15) {
              p_state->SOUTH_AREA_PW_FAILS++;
              printf("  INCORRECT PASSWORD!\n");
              printf("  %d ATTEMPTS REMAINING!\n\n",
                     16 - p_state->SOUTH_AREA_PW_FAILS);
            } else {
              printf("  You couldn't figure this out in under 16 attempts?...\n");
              printf("  The taco truck explodes sending your face in a firework display of gibs, salsa, and corn tortillas.\n\n");
              console_gfx_game_over();
              welcome_message();
              initialize_console_data();
              break;
            }
          }
        } else if ((strcmp(argv[1], "SD_SLOT") == 0) &&
                   (strcmp(argv[3], NORTH_AREA_ITEM) == 0) &&
                   (p_state->l00t[1].haz)) {
          printf("  The handy little EMMC_ADAPTER with a screen is coming to good use.\n");
          printf("  You wonder, who would make an EMMC_ADAPTER with a screen, seems kinda niche?\n");
          printf("  Well for the purposes of whats happening in this game, its mighty useful then...\n");
          printf("  The adapter's tiny screen display shows that once it is plugged in to the slot,\n");
          printf("  It fingerprints and recognizes that an x86 binary is being sent to it.\n");
          printf("  If only you knew where the SD card was in this Taco Truck that someone backed up a binary to.\n");
          printf("  More importantly, if only you knew how to analyze an x86 binary once you have the SD card...\n\n");
        } else error_flag = true;
      } break;

      case 3: {  // EAST AREA HACKS
        if ((strcmp(argv[1], "BIG_RED_BUTTON") == 0) &&
            (strcmp(argv[3], "FINGER") == 0) &&
            (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == false)) {
          printf("  You press the BIG_RED_BUTTON...\n");
          printf("  The %s begins to blink in some kind of pattern...\n\n",
                 EAST_AREA_ITEM);

          // Morse Code Display
          display_morse("....- ---..");  // 48
          display_morse("..--- ----.");  // 29
          display_morse("...-- -----");  // 30
          display_morse("..--- ....-");  // 24
          display_morse(".---- -....");  // 16
          display_morse(".---- ....-");  // 14
          display_morse(".---- .----");  // 11
          display_morse(".---- .----");  // 11
        } else if ((strcmp(argv[1], "D4RKM4TTER") == 0) &&
                   (strcmp(argv[3], SOUTH_AREA_ITEM) == 0) &&
                   (p_state->l00t[2].haz == true) &&
                   ((state_unlock_get() & UNLOCK_CACTUS) != 0x2000)) {
            // Unlock
            printf("  Once the cable is inserted it appears D4RKM4TTER begins to power back up...\n");
            printf("  Suddenly from the shadows, a dark gollum like figure dives in. Its ArdJect.\n");
            printf("  He begins performing what appears to be hacker forensics on the robo-corpse.\n");
            printf("  ArdJect then rips the USB_CABLE out, putting D4RKM4TTER back in to standby mode.\n");
            printf("  He throws it back at you then dives back into the shadows...WTAF?.\n");
            printf("  UNLOCK: You just got 666xp added to BOTNET!\n\n");
            state_unlock_set(state_unlock_get() | UNLOCK_CACTUS);
            state_botnet_get()->experience += 666;
        } else if ((strcmp(argv[1], "D4RKM4TTER") == 0) &&
                   (strcmp(argv[3], "FINGER") == 0)) {
            printf("  He robotically giggles...weird...\n\n");
        } else if ((strcmp(argv[1], "TERMINAL") == 0) &&
                   (strcmp(argv[3], "WIZARDRY") == 0) &&
                   (p_state->l00t[p_state->location].haz == false)) {
            printf(
            "  You entered the correct answer.\n"
            "  D4RKM4TTER falls to the ground, the power appears to be cut off.\n"
            "  It's probably rerouted down the cable you followed here.\n"
            "  His %s probably isn't of any use to him at this point.\n\n",
            EAST_AREA_ITEM);
            p_state->LOCATION_PUZZLE_SOLVED[p_state->location] = true;
        } else
          error_flag = true;
      } break;

      case 4: {  // WEST AREA HACKS
        if ((strcmp(argv[1], "APP") == 0) && (strcmp(argv[3], "FINGER") == 0) && (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == false)) {
          printf(
            "  The APP attempts to connect to the QUADCOPTER, but you haven't hacked in the password yet.\n"
            "  Not much to describe here, its not like you can see those invisible WiFi packets...\n"
            "  Standy by...this could take 10 seconds...\n");
          
          // Initialize & Start WiFi for the Puzzle
          wifi_config_t wifi_config = {
            .sta =
            {
                .ssid = CONSOLE_WIFI_SSID,
                .password = CONSOLE_WIFI_PASS,
            },
          };

          wifi_start(wifi_config);

          if (wifi_is_connected()) {
            // Connect to WebSite & Perform an HTTP Request
            puzzle_get_request();
            printf("  Your hacker wizardy lets you feel it, there are definitely packets floating in the air.\n");
            printf("  More importantly, the QUADCOPTER is connecting to the AP it expects & phoning home...\n\n");
          } else {
            printf("  You feel a great disturbance in the force, a lack of RF all around you.\n");
            printf("  The QUADCOPTER probably isn't even connecting to the AP it expects...\n\n");
          }

          // Stop the WiFi
          wifi_stop(); 
        } else if ((strcmp(argv[1], "APP") == 0) &&
                   (strcmp(argv[3], WEST_AREA_PASSWORD) == 0) &&
                   (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] ==
                    false)) {
          printf(
            "  The APP accepts the password!\n"
            "  VISS snatches the BURNER_PHONE back from you and takes control of the QUADCOPTER.\n"
            "  He lands it ever gracefully on the bar top...but then something unexpected happens...\n"
            "  April, still pissed off by the rampant buzzing, takes a bar shaker and smashes the mini QUADCOPTER.\n"
            "  Broken bits of electronics now scatter the bar top. \n"
            "  It's Centered around a metallic bar shaker filled with MILKSHAKE.\n"
            "  VISS tosses the BURNER_PHONE on the floor, now that he has no use for it.\n"
            "  The only good news is now that the QUADCOPTER is done for, it appears the WiFi is once again working.\n"
            "  Stupid cheap little QUADCOPTER was most likely part of an IoT botnet and using all the bandwidth.\n"
            "  The WiFi power meter outside the bar begins phoning home.\n"
            "  Blinky lights on the meter indicate power is being supplied to the cable you followed!\n\n");

          p_state->LOCATION_PUZZLE_SOLVED[p_state->location] = true;
        } else if ((strcmp(argv[1], "QUADCOPTER") == 0) &&
                   (strcmp(argv[3], EAST_AREA_ITEM) == 0) &&
                   (p_state->l00t[3].haz) &&
                   (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] ==
                    false)) {
          printf(
              "  Your sharp wits and instincts were on to something.\n"
              "  The %s starts collecting and chewing on packets.\n"
              "  The status output shows the QUADCOPTER is trying to connect to an open access point named: DRONELIFE\n"
              "  Clever girl...\n\n",
              EAST_AREA_ITEM);
        } else if ((strcmp(argv[1], "QUADCOPTER") == 0) &&
                   (strcmp(argv[3], EAST_AREA_ITEM) == 0) &&
                   (p_state->l00t[3].haz) &&
                   (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] ==
                    true)) {
          printf(
              "  Your sharp wits and instincts were on to nothing.\n"
              "  The QUADCOPTER is smashed to bits, remember? You are definitely not on your game...\n\n");
        } else if ((strcmp(argv[1], "VISS") == 0) &&
                   (strcmp(argv[3], "FINGER") == 0)) {
          printf("  Why the fuck would you poke poor Dan?...\n\n");
        } else if ((strcmp(argv[1], "MILKSHAKE") == 0) &&
                   (strcmp(argv[3], "COCKROACH") == 0) && (p_state->TREVOR) &&
                   (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == true) &&
                   ((state_unlock_get() & UNLOCK_TREVOR) != 0x0800)) {
          console_gfx_area_eggz(p_state->location);  // Trevor ANSI Art
          printf("  #TREVORFORGET...Bling Unlocked!\n\n");
          p_state->TREVOR = false;
          state_unlock_set(state_unlock_get() | UNLOCK_TREVOR);
        } else
          error_flag = true;
      } break;
    }
  } else
    error_flag = true;

  if (error_flag) {
    printf("  Am I doing it right? NO! You're not doing it right...\n\n");
  }
  return 0;
}

static void register_hack() {
  hack_args.thingz =
      arg_str0(NULL, NULL,
               " THING_YOU_SEE with (THING_IN_L00T || INTEL_YOU_KNOW) ", NULL);
  hack_args.end = arg_end(1);

  const esp_console_cmd_t cmd = {.command = "hack",
                                 .help = "Hack all the thingz...",
                                 .hint = NULL,
                                 .func = &hack,
                                 .argtable = &hack_args};
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int look(int argc, char** argv) {
  game_data_t* p_state = state_console_get();
  bool error_flag = false;

  switch (argc) {
    case 1: {  // displays information about the environment to the user - user
      // typed "look"
      switch (p_state->location) {
        case 0:  // HOME - PUZZLE REQUIRING MATH SKILLS, CALCULATOR, AND/OR SPREADHEET (MATHEMATICS)
          if (p_state->HOME_AREA_COMPLETE == false) {
            printf(
                "  You are standing in the Center of two intersecting trails going farther than the eye can see \n"
                "  They are perpendicular creating four possible directions: NORTH, SOUTH, EAST, and WEST.\n"
                "  There is a shaded pop up ten covering a TERMINAL on a table and a locked hatch on the ground.\n"
                "  Four large cables, running along each trail, connect to the back of a server rack next to the table.\n");
            if (p_state->l00t[p_state->location].haz == false)
              printf("  On the table you also you see an empty IoT enabled VODKA_BOTTLE and a %s.\n", HOME_AREA_ITEM);
            else
              printf("  On the table you also you see an empty IoT enabled VODKA_BOTTLE.\n");
            printf(
                "  You wonder, who the hell would put such a thing in the desert?\n"
                "  Lets make the best of this situation and call this new place HOME.\n\n");
          } else {
            if (p_state->GAME_WON == false) {
              printf("  You are in a multi-colored blinky lit room from dozens of BADGES.\n");
              printf("  One stands out a HYPERCUBE_BADGE.\n");
              printf("  Aside from that, the floor is littered with used salsa cups and Taco Corp snack boats...\n");
              printf("  At the opposite end of the room is a door with a glowing EXIT_BUTTON.\n\n");
            } else {
              printf("  TO BE CONTINUED...\n\n");
              printf("  YOU HAVE COMPLETED THE AND!XOR DC26 BADGE CHALLENGE.\n");
              printf("  CONTACT US ON TWITTER @ANDNXOR && EMAIL HYR0N@ANDNXOR.COM ASAP BEFORE THE CON IS OVER.\n");
              printf("  YOUR STATE WILL SAVE ON A TASK THAT RUNS EVERY 1 MIN, YOU MAY SAFELY DISCONNECT THE BADGE IN 1 MIN...\n\n");
            }
          }
          break;

        case 1:  // NORTH - PUZZLE REQUIRING A LOGIC ANALYZER OR O-SCOPE (HARDWARE HACKING)
          printf(
              "  You are at the NORTH location.\n"
              "  It's an abandoned roadside strip mall, most everything is boarded up, except the Radio Shack.\n"
              "  You follow the cable in to the Radio Shack, cuz...its 80's Radio Shack, duh. Cool shit.\n"
              "  The smell of flux, magic smoke, and lead solder fills the air.\n");
          if (((state_unlock_get() & UNLOCK_TREVOR) == 0x0800)||(p_state->TREVOR))
            printf("  There are scattered electronic components on the floor, a PHONE_RECYCLE_BIN, and an empty taco wrapper.\n");
          else
            printf("  There are scattered electronic components on the floor, a PHONE_RECYCLE_BIN, as well as a dead COCKROACH.\n");
          if (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == false)  // Haven't beat the puzzle yet
            printf(
                "  To your surprise, the place is powered by Joe Grand...running on a treadmill...\n"
                "  and like most runners he seems pretty damn stoked about it.\n"
                "  Maybe its also because he setup a soldering station, boards, BADGES, and an o-scope on the treadmill table.\n"
                "  What do you expect? Hardware hackers gonna hack...especially in Radio Shack.\n"
                "  On the side of the treadmill is a junction box where the cable runs in and a PIN_PAD on the front.\n"
                "  You tell Joe your predicament, how you need to power the rack back home.\n"
                "  He replies while still on the treadmill...\n\n"
                "  CANT...STOP...RUNNING...BUT...NO...WORRIES...\n"
                "  PIN_PAD...NEED...CARBS...SORRY...DONT...KNOW...PIN...RANDOM...\n"
                "  ONLY...TWO...BYTES...WORTH...OF...BITS...TO...TRI...\n"
                "  OR...WE...LOSE...RACE...\n"
                "  BE...LOGICIAL...URGH...SIDE...ACHE...CHANNEL...EFFORTS...\n"
                "  CHECK...CADENCE...POWER...THRU...\n"
                "  I...TOO...SEE...FINISH...LINE...\n"
                "  CADENCE...REST...GETS...LONGER...CLOSER...YOU...GET...\n"
                "  DONT...MESS...IT...UP...LEGS...EXPLODE...\n\n");
          else {
            printf(
                // Standard context displayed once the puzzle has been beaten
                "  Joe continues to run on the treadmill, sending power down the cable back to HOME.\n");
            if (p_state->l00t[p_state->location].haz == false) {
              // But not yet have looted
              printf("  The %s sits under the junction box.\n\n", NORTH_AREA_ITEM);
            } else {
              // Haz looted
              printf("  The junction box is no longer of any interest.\n\n");
            }
          }
          break;

        case 2:  // SOUTH - PUZZLE REQUIRING STATIC BINARY ANALYSIS (REVERSE ENGINEERING)
          printf(
              "  You are at the SOUTH location.\n"
              "  The trail lead to a vast empty parking lot with an abandoned Taco Corp truck.\n"
              "  There is a MENU on the side of the truck, partially legible.\n"
              "  The cable you followed here leads to the truck, interfaced with its diesel generator.\n"
              "  You peer in through the window and see a mobile kitchen which was recently used.\n"
              "  There are many types of SALSA, still fresh, in nearly a dozen or so serving vats.\n");
          if (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == false)  // Haven't beat the puzzle yet
            printf(
                "  This is definetly legit, given there are more varieties of SALSA than food on the menu.\n"
                "  The TOUCHSCREEN computer register is powered on and most likely controls everything in the truck.\n"
                "  Someone you need to get into that computer and turn on the generator...\n\n");
          else {
            printf(
                // Standard context displayed once the puzzle has been beaten
                "  The generator humms loudly with the power of tacos.\n"
                "  Habenero hot electrons are definetly being sent back to the HOME village.\n");
            if (p_state->l00t[p_state->location].haz == false) {
              // But not yet have looted
              printf(
                  "  The %s isn't probably isn't useful to the TOUCHSCREEN register anymore.\n", SOUTH_AREA_ITEM);
              printf(
                  "  It's just connecting a credit card reader and all the cool kids use DEFCOIN anyway.\n\n");
            } else {
              // Haz looted
              printf(
                  "  Power supplied and l00ted up. Unless there are more tacos, ditch this place...\n\n");
            }
          }
          break;

        case 3:  // EAST - PUZZLE REQUIRING ONLY YOUR BRAINS (CRYPTOGRAPHY)
          printf(
              "  You are at the EAST location.\n"
              "  The trail ended in a forest of Joshua trees and cactus.\n"
              "  The cable snakes through the forest to the entrance of a cave.\n");
          if (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == false)  // Haven't beat the puzzle yet
            printf(
                "  From inside, sounds like the echoing music of the electronic type. You decide to enter the cave.\n"
                "  The cave is lit with a neon green hue and thin shadows dance across the wall.\n"
                "  While your eyes adjust, what appears to be a cactus dancing to the music...\n"
                "  Comes into focus as D4RKM4TTER dancing to EDM, rigged up with his %s.\n"
                "  However, this does not look right. He appears to be more of a cyborg zombie.\n"
                "  The large cable you followed runs into a computer TERMINAL with a BIG_RED_BUTTON. \n"
                "  Another cable leaves the TERMINAL housing and into the back of the skull of D4RKM4TTER.\n"
                "  It appears he doesn't notice, or care that you are there, only stuck in an endless dancing loop.\n"
                "  He is also dancing with a taco in each hand...unsure if stoked about tacos or EDM...\n"
                "  With no WiFi packets to capture in this cave...he is without purpose.\n\n"
                "  On the wall behind the terminal you see a message spelled out in googly eyes:\n\n"
                "  SO ASK YOURSELF BURRP THESE BLINGED OUT BADGES\n"
                "  HOW DO THEY WORK IS IT THE BUURRRP ELECTRONS\n"
                "  IS IT THE SOURCE CODE OR THE BUUURRP BOOZE\n"
                "  SOMETIMES SCIENCE IS MORE ART THAN SCIENCE MORTY\n"
                "  LOT OF PEOPLE DONT GET THAT\n"
                "  RICK SCHWIFTY SANCHEZ\n\n",
                EAST_AREA_ITEM);
          else {
            printf(
                // Standard context displayed once the puzzle has been beaten
                "  Inside the cave, it is now silent and dark, only light from the outside bleeding in.\n"
                "  The power appears to be cut off, and probably rerouted down the cable you followed from HOME.\n"
                "  The hacking dead, D4RKM4TTER, appears to have some residual energy left, but not much to be of use.\n"
                "  Strange...that D4RKM4TTER was actually some necromantic cyborg hacking zombie...explains...a lot.\n"
                "  Carrying around all that networking equipment and car batteries was just a front.\n"
                "  Now he's just an underpowered pile of parts, booz3, tacos, and USB ports.\n");
            if (p_state->l00t[p_state->location].haz == false) {
              // But not yet have looted
              printf(
                  "  His %s isn't probably of any use to him at this point.\n\n", EAST_AREA_ITEM);
            } else {
              // Haz looted
              printf(
                  "  You boldy sport the %s he once wore...damn this shit is heavy...\n\n", EAST_AREA_ITEM);
            }
          }
          break;

        case 4:  // WEST - PUZZLE REQUIRING WIRESHARK AND MONITOR MODE CAPABLE WIFI ADAPTER (PCAP & WIRELESS HACKING)
          printf(
              "  You are at the WEST location.\n"
              "  A western ghost-town style saloon welcomes you with open doors.\n"
              "  A familiar face is bartending craft beers and cosmos, April Wright, no surprise there.\n"
              "  Bartending, the perfect cover & complement to OSINT.\n");
          if (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] ==
              false)  // Haven't beat the puzzle yet
            printf(
                "  She gives you the hush hush look to not blow it.\n"
                "  However there's another person in the bar who stands out more than others, Viss.\n"
                "  He is cursing at his out of control QUADCOPTER. Apparently it's been hijacked somehow.\n"
                "  To be fair, it was a hacked Taco Corp taco delivery drone to begin with...\n"
                "  The QUADCOPTER is just buzzing around the bar, pissing off April with its noise...\n"
                "  Probably because it interfers with her bar top intelligence gathering.\n"
                "  At least its only one of those cheap WiFi knock off brands. \n"
                "  If only you could help him regain control.\n"
                "  His BURNER_PHONE has an APP which controls it, but he needs the password.\n"
                "  Apparently all you can do is hack the APP with your FINGER to try and connect.\n\n");
          else {
            printf(
                // Standard context displayed once the puzzle has been beaten
                "  VISS sits at the bar drinking, sad, that his drone was smashed.\n"
                "  In the middle of the smashed bits sits the evil death bringing MILKSHAKE.\n");
            if (p_state->l00t[p_state->location].haz == false) {
              // But not yet have looted
              printf("  The BURNER_PHONE sits on the bar floor, it's of no use to VISS now...\n\n");
            } else {
              // Haz looted
              printf("  Other than that, the power meter is spinning and power is being supplied back HOME.\n\n");
            }
          }
          break;
      }
    } break;

    // displays details information about items you are looking at up close,
    // switch filtered by location - they typed "look at THING"
    case 3: {
      if (strcmp(argv[1], "at") ==
          0) {  // make sure they at least type "at" and not some garbage
        switch (p_state->location) {
          case 0: {  // Home Items
            if (p_state->HOME_AREA_COMPLETE == false) {
              if (strcmp(argv[2], "VODKA_BOTTLE") == 0) {
                printf(
                    "  You think to yourself, fuck this empty bottle of b00z3... \n"
                    "  However notice the bottle is still scrolling a message: \n"
                    "  800618943025315f869e4e1f09471012\n"      // F
                    "  a87ff679a2f3e71d9181a67b7542122c\n"      // 4
                    "  8d9c307cb7f3c4a32822a51922d1ceaa\n"      // N
                    "  0d61f8370cad1d412f80b84d143e1257\n"      // C
                    "  dd7536794b63bf90eccfd37f9b147d7f\n"      // I
                    "  eccbc87e4b5ce2fe28308fd9f2a7baf3\n"      // 3
                    "  6966277c536759a4bcc3c0a4e1eaa160\n"      // DRANK
                    "  56e591f0c8e18fcd60303753c16b7a43\n"      // EVERYTHING
                    "  ede57168ad3a5db2c29903150fd6f950\n"      // MAKING
                    "  6222ba386bcc675dab6063afba7235f1\n"      // FANCY
                    "  ee3f0f452b9342ec4b6d0e3fb54ecb01\n"      // BOATS
                    "  2c2624a5059934a947d6e25fe8332ade\n"      // FIRST
                    "  c86ee0d9d7ed3e7b4fdbf486fa6c0ebb\n"      // IN
                    "  45ac78bf3d4882ac520f4e7fb08d55c5\n"      // EAST
                    "  907ec71a28d71811a0e37f08b15c2109\n"      // THEN
                    "  2bf8f791695c70efa9c14e6f1c326403\n\n");  // NORTH
              } else if (strcmp(argv[2], "TERMINAL") == 0) {
                // Check to see if all items have finally been collected
                int puzzle_win_count = 0;
                for (int i = 0; i <= 4; i++) {
                  if (p_state->LOCATION_PUZZLE_SOLVED[i] == true)
                    puzzle_win_count++;
                }

                printf(
                    "  The terminal is on with the soothing green glow of a monochrome monitor.\n"
                    "  The screen reads...\n\n"
                    "  TO OPEN THE HATCH YOU MUST ABIDE BY THE FOLLOWING RULES:\n"
                    "  * HAVE A BLOOD ALCOHOL LEVEL OF EXACTLY 0.1337, ACCURATE TO 4 DECIMAL PLACES\n"
                    "  * DRINK FROM ALL FIVE CRAFT BEERS COLLECTED\n"
                    "  * %% BAC = ((A x 5.14) \\ (W x R)) – 0.015 x H\n"
                    "  * A = TOTAL LIQUID OUNCES OF ALCOHOL CONSUMED = ABV * VOLUME OF ALCOHOL DRANK\n"
                    "  * W = PERSONS WEIGHT IN POUNDS\n"
                    "  * R = GENDER CONSTANT OF ALCOHOL DISTRIBUTION (0.73 FOR MALE, 0.66 FOR FEMALE, 0.695 FOR NON-BINARY)\n"
                    "  * H = HOURS ELAPSED SINCE YOUR FIRST DRINK (HOPE YOU DIDNT START THE PARTY EARLY)\n\n");

                if (puzzle_win_count == 5) {
                  printf(
                      "  Suddenly the door to the computer rack swings open and a shiny metal bending robot emerges.\n"
                      "  EllwoodTheWood's BREATHALYZER badge hangs from below it's waist...\n"
                      "  The robot yell's in a surly tone \"GET DRUNK AND BLOW ME!\"\n"
                      "  (hint: hack BREATHALYZER with BREATH)\n\n");
                } else {
                  printf(
                      "  You hear a low pitch whirring and bangs on the rack door, as if something is trying to get out.\n"
                      "  Whatever it is, it probably doesn't have enough power to do so.\n\n");
                }
              } else
                error_flag = true;
            } else if (p_state->HOME_AREA_COMPLETE == true) {
              if (strcmp(argv[2], "HYPERCUBE_BADGE") == 0) {
                printf("  A beautiful cube shaped conference badge.\n");
                printf("  The creator of such an electronic piece of art is on a different level from the rest of us.\n");
                printf("  You can't resist the urge to touch it...\n\n");
              } else if (strcmp(argv[2], "EXIT_BUTTON") == 0) {
                printf("  A big red button you can push that supposidly opens the exit.\n");
                printf("  Can't beat that!\n\n");
              } else if (strcmp(argv[2], "BADGES") == 0) {
                printf("  A relic from DC25 stands out...\n\n");
                console_gfx_area_eggz(0);
              } else
                error_flag = true;
            } else
              error_flag = true;
          } break;

          case 1: {  // North Items
            if (strcmp(argv[2], "PIN_PAD") == 0) {
              printf(
                  "  A PIN_PAD with only the numbers 1,2,3, and 4.\n"
                  "  The display shows only a permutation of four digits can be entered.\n"
                  "  Somehow you need to hack PIN_PAD with the correct four digits ####\n\n");
            } else if (strcmp(argv[2], "COCKROACH") == 0) {
              printf("  Poor little guy...#TrevorForget\n\n");
            } else if (strcmp(argv[2], "BADGES") == 0) {
              console_gfx_area_eggz(1);  // G-Board Bling
              printf(
                  "  A G shaped hardware hacking badge...could this be responsible for all of this badgelife?\n\n");
            } else if (strcmp(argv[2], "PHONE_RECYCLE_BIN") == 0) {
              printf(
                  "  One of those cardboard recycling dropboxes for old cell phones.\n\n");
            } else
              error_flag = true;
          } break;

          case 2: {  // South Items
            if (strcmp(argv[2], "MENU") == 0) {
              printf(" The menu is barely legible due to GRAFITTI.\n");
              printf(" You can barely make out the writing of carne asada, carnitas, and shrimp tacos.\n\n");
            } else if (strcmp(argv[2], "GRAFITTI") == 0) {
              console_gfx_area_eggz(p_state->location);
              printf("  The grafitti is interesting, looks useful and techno-logical-mo-der-ne. \n\n");
            } else if (strcmp(argv[2], "SALSA") == 0) {
              printf("  The life blood of the world which we know as mexican food.\n");
              printf("  Tomatillo, Habenero, Rojo, Chipotle, Fresca, Taquera.\n");
              printf("  No one is around to tell if you stick your FINGER in for a taste...\n\n");
            } else if (strcmp(argv[2], "TOUCHSCREEN") == 0) {
              printf("  A touchscreen x86 based computer running a register and all the taco truck gadgets.\n");
              printf("  Definetly a hacker mod, empty SD_SLOT, and looks like its running Linux or BSD. Then you think...\n");
              printf("  No one in the world uses BSD because its a hot pile of garbage.\n");
              printf("  Regardless, this computer is password protected to the max.\n");
              printf("  You also notice before the password prompt it has a recent system log message:\n");
              printf("  CRON-201808051337:Taco Corp TacOS Authorization Backup to SD Complete.\n\n");
            } else if (strcmp(argv[2], "SD_SLOT") == 0) {
              printf("  An empty little SD_SLOT...something could possibly go in there...to teach you thingz...about thingz...\n\n");
            } else error_flag = true;
          } break;

          case 3: {  // East Items
            if (strcmp(argv[2], "TERMINAL") == 0) {
              printf("  The terminal sits with an vacant input prompt.\n");
              printf("  HOW DOES THE BADGE WORK?>\n\n");
            } else if (strcmp(argv[2], "BIG_RED_BUTTON") == 0) {
              printf("  No l337 h@x04 can resist exploiting the vulnerability of an unprotected a big red button.\n");
              printf("  Life hack it with your finger...\n\n");
            } else if (strcmp(argv[2], EAST_AREA_ITEM) == 0) {
              console_gfx_area_eggz(3);
              printf(
                  "  25 Hak5 Pineapple Tetra routers, an Intel NUC, 2 Cisco 16 port switches, a 500-watt 12v power supply, \n"
                  "  a 10-amp 12v to 5v DC converter, an Arduino Micro, a 30-amp hour battery, and 120 neopixels.\n"
                  "  You think to yourself, why did it have to be Neopixels...sign of the LEDevil...\n\n");
            } else
              error_flag = true;
          } break;

          case 4: {  // West Items
            if ((strcmp(argv[2], "QUADCOPTER") == 0) &&
                (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == false)) {
              printf(
                  "  An inexpensive little WiFi enabled taco delivery drone.\n"
                  "  It's 3d printed quality looks to be 100%% IoT Goodness.\n\n");
            } else if ((strcmp(argv[2], "QUADCOPTER") == 0) &&
                       (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == true)) {
              printf(
                  "  An inexpensive smashed little once WiFi enabled taco delivery drone.\n"
                  "  It's internal electronic guts look to be 100%% chineesium.\n\n");
            } else if (strcmp(argv[2], "BURNER_PHONE") == 0) {
              printf(
                  "  It's one of the smallest Android burners you've ever seen.\n"
                  "  So small, it could fit...weird...it kind of smells like ass?\n\n");
            } else if (strcmp(argv[2], "APP") == 0) {
              printf(
                  "  A simple smartphone app with directional keys. Hard to see on this tiny phone.\n"
                  "  WiFi is on but how the hell would you know what AP it's trying to connect to?\n"
                  "  If only there were some way to grab those WiFi packets out of the air and inspect them.\n"
                  "  Then you just might be able to hack the APP with whatever password it's waiting for...\n\n");
            } else if ((strcmp(argv[2], "MILKSHAKE") == 0) && (p_state->LOCATION_PUZZLE_SOLVED[p_state->location] == true)) {
              printf("  A bar shaker filled with a creamy strawberry milkshake.\n\n");
            } else error_flag = true;
          } break;
        }
      } else
        error_flag = true;
      break;
    } break;

    default:
      error_flag = true;
      break;
  }

  if (error_flag)
    printf(
        "  That's not something you can look at...drunky Mc Drunk face...\n\n");

  return 0;
}

static void register_look() {
  const esp_console_cmd_t cmd = {
      .command = "look",
      .help = "Look around your general area or at CAPITALIZED_ITEMS",
      .hint = "look <no args> OR look at <XXX> {detailed information about an item}",
      .func = &look,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

static int l00t(int argc, char** argv) {
  game_data_t* p_state = state_console_get();
  /** List the contents of all dat sweet sweet l00t*/
  bool any_l00t = false;
  for (int i = 0; i <= 4; i++) {
    if (p_state->l00t[i].haz) {
      printf("  You haz a %s \n", p_state->l00t[i].name);
      printf("  You haz a bomber of %s ", p_state->l00t[i].beer.name);
      printf("(%.2f oz remaining) \n\n", p_state->l00t[i].beer.volume);
      any_l00t = true;
    }
  }

  if (p_state->TREVOR)
    any_l00t = true;

  if (any_l00t == true) {
    if (p_state->TREVOR)
      printf("  You haz a dead COCKROACH\n\n");
    printf("  You haz a l337 hax0r FINGER...\n\n");
  }

  if (any_l00t == false) {
    printf(
        "  You don't haz any l00t yet...except your l337 hax0r FINGER...\n\n");
  }
  return 0;
}

static void register_l00t() {
  const esp_console_cmd_t cmd = {
      .command = "l00t",
      .help = "List the contents of all dat sweet sweet l00t",
      .hint = NULL,
      .func = &l00t,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/**
 * @brief Execute ps system command by console. Dump tasks.
 */
static int system_name(int argc, char** argv) {
  char name[STATE_NAME_LENGTH + 1];

  // No arguments
  if (argc == 1) {
    state_name_get(name);
    printf("  %s\n\n", name);
  } else if (argc == 2) {
    // Ensure input is safely truncated
    uint8_t len = MIN(STATE_NAME_LENGTH, strlen(argv[1]));
    snprintf(name, len + 1, "%s", argv[1]);

    // Ensure input is in valid range
    bool invalid = false;
    for (uint8_t i = 0; i < len; i++) {
      if (name[i] < INPUT_CHAR_MIN) {
        name[i] = ' ';
        invalid = true;
      } else if (name[i] > INPUT_CHAR_MAX) {
        name[i] = ' ';
        invalid = true;
      }
    }

    // If they input at least one invalid character be a little mean
    if (invalid) {
      sprintf(name, "  LOLOL:-)");
    }

    // Store the name
    state_name_set(name);
    state_save_indicate();

		printf("  Name set to '%s'\n\n", name);
  } else {
    printf("  Invalid arguments.");
  }
  return 0;
}

/**
 * @brief Execute free memory command by console
 */
static int system_memory(int argc, char **argv) {
   printf("Free Heap: %d\t"
           "Lowest Heap: %d\t"
           "Free DMA: %d\n\n",
           xPortGetFreeHeapSize(), 
           xPortGetMinimumEverFreeHeapSize(), 
           heap_caps_get_free_size(MALLOC_CAP_DMA));
  return 0;
}

/**
 * @brief Execute peers system command by console
 */
static int system_peers(int argc, char** argv) {
  peers_dump();
  return 0;
}

/**
 * @brief Execute ps system command by console. Dump tasks.
 */
static int system_ps(int argc, char** argv) {
  util_task_stat_dump();
  return 0;
}

/** 
 * @brief Register free memory command in the console
 */
static void system_register_memory() {
  const esp_console_cmd_t cmd = {
    .command = "free",
    .help = "Print free memory",
    .hint = NULL,
    .func = &system_memory,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/**
 * @brief Register ps command in the console
 */
static void system_register_name() {
  const esp_console_cmd_t cmd = {
      .command = "whoami",
      .help = "Print or change user name",
      .hint = NULL,
      .func = &system_name,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/**
 * @brief Register peers system command in console
 */
static void system_register_peers() {
  const esp_console_cmd_t cmd = {
      .command = "peers",
      .help = "Print a list of nearby peers",
      .hint = NULL,
      .func = &system_peers,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}

/**
 * @brief Register ps command in the console
 */
static void system_register_ps() {
  const esp_console_cmd_t cmd = {
      .command = "ps",
      .help = "Print currently running tasks",
      .hint = NULL,
      .func = &system_ps,
  };
  ESP_ERROR_CHECK(esp_console_cmd_register(&cmd));
}