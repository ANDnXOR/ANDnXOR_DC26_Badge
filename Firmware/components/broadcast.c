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

#include "system.h"

__attribute__((__unused__)) const static char* TAG = "MRMEESEEKS::Broadcast";

static broadcast_packet_t m_broadcast_packet;
static broadcast_t m_last_broadcast_received;
static uint32_t m_next_broadcast_timestamp = 0;

/**
 * @brief Process broadcast packets received. Packets may still be invalid and
 * must be further checked and handled
 *
 * @param p_packet : Pointer to the parsed and decrypted broadcast packet
 * @param p_peer : Pointer to the known peer that broadcast the message
 */
void broadcast_advertisement_handler(broadcast_packet_t* p_packet,
                                     peer_t* p_peer) {
  if (p_packet == NULL) {
    return;
  }

  // ESP_LOGD(TAG, "%s Received timestamp=%ld last timestamp=%ld", __func__,
  //          p_packet->timestamp, m_last_broadcast_received.timestamp);

  // First thing, check the CRC
  uint16_t crc =
      crc16_le(0, (uint8_t*)p_packet, sizeof(broadcast_packet_t) - 2);
  if (crc != *(uint16_t*)p_packet->crc) {
    ESP_LOGD(TAG, "%s CRC Invalid :(", LOG_RED);
    return;
  }

#ifdef CONFIG_DEBUG_BLE
  ESP_LOGD(TAG, "RECV TS: %d Local TS: %d", *(uint32_t*)p_packet->timestamp,
           time_manager_now_sec());
#endif

  // If received broadcast is later than last broadcast, lets do some stuff and
  // things
  if (*(uint32_t*)p_packet->timestamp > m_last_broadcast_received.timestamp) {
    // Wipe the last broadcast
    memset(&m_last_broadcast_received, 0, sizeof(broadcast_t));

    // Copy the broadcast locally
    memcpy(&m_last_broadcast_received.name, p_peer->name, STATE_NAME_LENGTH);
    memcpy(&m_last_broadcast_received.message, p_packet->message,
           BROADCAST_MESSAGE_LENGTH);
    m_last_broadcast_received.timestamp = *(uint32_t*)p_packet->timestamp;

    // Interrupt if rate limiting allows
    ESP_LOGD(TAG,
             "Comparing broadcast timestamp of %d to next %d for interrupt",
             m_last_broadcast_received.timestamp, m_next_broadcast_timestamp);
    if (m_last_broadcast_received.timestamp > m_next_broadcast_timestamp) {
      ui_interrupt(m_last_broadcast_received.message,
                   "/sdcard/bling/cowboy.raw", 15);
      m_next_broadcast_timestamp =
          time_manager_now_sec() + (CONFIG_BROADCAST_RATE_LIMIT / 1000);
      ESP_LOGD(TAG, "Next timestamp set to %d", m_next_broadcast_timestamp);
    }
    // ESP_LOGD(TAG, "%s: %s (%ld)", m_last_broadcast_received.name,
    //          m_last_broadcast_received.message,
    //          m_last_broadcast_received.timestamp);
  }
}

void broadcast_init() {
  // Make sure current packet is blank
  memset(&m_broadcast_packet, 0, sizeof(broadcast_packet_t));
  // Calculate an initial CRC
  *(uint16_t*)m_broadcast_packet.crc = crc16_le(
      0, (uint8_t*)&m_broadcast_packet, sizeof(broadcast_packet_t) - 2);

  // init last broadcast received to nothing
  memset(&m_last_broadcast_received.message, 0, BROADCAST_MESSAGE_LENGTH + 1);
  memset(&m_last_broadcast_received.name, 0, STATE_NAME_LENGTH + 1);
  m_last_broadcast_received.timestamp = 0;
}

/**
 * @brief Get the current broadcast packet being sent
 * @return The current broadcast packet
 */
broadcast_packet_t* broadcast_get() {
  // If CRC is 0, we should init
  if (*(uint16_t*)m_broadcast_packet.crc == 0) {
    broadcast_init();
  }

#ifdef CONFIG_BADGE_TYPE_STANDALONE
  sprintf(m_broadcast_packet.message, "Testing");
  *(uint32_t*)m_broadcast_packet.timestamp = time_manager_now_sec();
  *(uint16_t*)m_broadcast_packet.crc = crc16_le(
      0, (uint8_t*)&m_broadcast_packet, sizeof(broadcast_packet_t) - 2);
#endif
  return &m_broadcast_packet;
}

void broadcast_last_received_get(broadcast_t* p_broadcast) {
  memcpy(p_broadcast, &m_last_broadcast_received, sizeof(broadcast_t));
}

void broadcast_set(char* message) {
  if (strlen(message) > BROADCAST_MESSAGE_LENGTH) {
    message[BROADCAST_MESSAGE_LENGTH] = 0;
  }
  // If CRC is 0, we should init
  if (*(uint16_t*)m_broadcast_packet.crc == 0) {
    broadcast_init();
  }

  // Store the broadcast message with current timestamp
  sprintf(m_broadcast_packet.message, message);
  *(uint32_t*)m_broadcast_packet.timestamp = time_manager_now_sec();

  // Calculate a CRC
  *(uint16_t*)m_broadcast_packet.crc = crc16_le(
      0, (uint8_t*)&m_broadcast_packet, sizeof(broadcast_packet_t) - 2);
}
