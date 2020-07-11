/*
 * Copyright 2020 Andrew Rossignol andrew.rossignol@gmail.com
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
 */

#include "net/aprs_interface.h"

#include "util/callsign.h"
#include "util/log.h"
#include "util/string.h"
#include "util/time.h"

#define LOG_TAG "APRSInterface"

namespace au {

APRSInterface::APRSInterface(const Config& config)
    : config_(config),
      next_payload_id_(GetTimeNowUs() & 0xffffffff) {}

bool APRSInterface::SendBroadcastPacket(const Packet& packet,
    const CallsignConfig& source,
    const std::vector<CallsignConfig>& digipeaters) {
  const CallsignConfig kBroadcastDestination({kBroadcastCallsign, 0});

  std::string serialized_packet;
  if (!packet.SerializeToString(&serialized_packet)) {
    LOGFATAL("failed to serialize packet");
  }

  uint32_t payload_id = GetNextPayloadId();
  LOGI("sending payload_id %zu", payload_id);

  uint32_t chunk_id = 1;
  uint64_t next_packet_time_us = GetTimeNowUs();
  for (uint32_t offset = 0; offset < serialized_packet.size();) {
    PacketChunk packet_chunk;
    auto* chunk = packet_chunk.mutable_chunk();
    chunk->set_payload_id(payload_id);
    chunk->set_chunk_id(chunk_id++);
    if (offset == 0) {
      chunk->set_total_payload_size(serialized_packet.size());
    }

    size_t chunk_size = std::min(config_.max_packet_size,
        serialized_packet.size() - offset);
    chunk->set_payload(serialized_packet.substr(offset, chunk_size));

    if (!SendPacketChunk(packet_chunk, source, kBroadcastDestination,
          digipeaters)) {
      LOGE("failed to send packet chunk");
      return false;
    }

    LOGI("sent broadcast chunk_id=%zu, offset=%zu, chunk_size=%zu, "
        "total_size=%zu", chunk->chunk_id(), offset, chunk_size,
        serialized_packet.size());
    offset += chunk_size;

    // Pause for the next transmission.
    next_packet_time_us += config_.transmit_interval_s * kUsPerS;
    SleepUntil(next_packet_time_us);
  }

  return true;
}

uint32_t APRSInterface::GetNextPayloadId() {
  uint32_t next_payload_id = next_payload_id_++;
  if (next_payload_id == 0) {
    next_payload_id = next_payload_id_++;
  }

  return next_payload_id;
}

bool APRSInterface::SendPacketChunk(const PacketChunk& chunk,
    const CallsignConfig& source, const CallsignConfig& destination,
    const std::vector<CallsignConfig>& digipeaters) {
  std::string serialized_chunk;
  if (!chunk.SerializeToString(&serialized_chunk)) {
    LOGFATAL("failed to serialize chunk");
  }

  std::string base64_serialized_chunk = StringBase64Encode(serialized_chunk);
  std::string aprs_packet = "{" + base64_serialized_chunk;
  return Send(aprs_packet, source, destination, digipeaters);
}

}  // namesapce au
