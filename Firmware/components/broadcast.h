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
#ifndef COMPONENTS_BROADCAST_H_
#define COMPONENTS_BROADCAST_H_

#include "system.h"

// Limit broadcast to 16-bytes
#define BROADCAST_MESSAGE_LENGTH 10

// Must be exactly 16 bytes to fit into AES ECB
typedef struct {
  char message[BROADCAST_MESSAGE_LENGTH];
  uint8_t timestamp[4];
  uint8_t crc[2];  // Must be last
} broadcast_packet_t;

typedef struct {
  char name[STATE_NAME_LENGTH + 1];
  char message[BROADCAST_MESSAGE_LENGTH + 1];
  uint32_t timestamp;
} broadcast_t;

/**
 * @brief Process broadcast packets received. Packets may still be invalid and
 * must be further checked and handled
 *
 * @param p_packet : Pointer to the parsed and decrypted broadcast packet
 * @param p_peer : Pointer to the known peer that broadcast the message
 */
extern void broadcast_advertisement_handler(broadcast_packet_t* p_packet,
                                            peer_t* p_peer);

/**
 * @brief Get the current broadcast packet being sent
 * @return The current broadcast packet
 */
extern broadcast_packet_t* broadcast_get();
extern void broadcast_init();
extern void broadcast_last_received_get(broadcast_t* p_broadcast);
extern void broadcast_set(char* message);

#endif /* COMPONENTS_BROADCAST_H_ */
