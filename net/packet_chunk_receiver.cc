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

#include "net/packet_chunk_receiver.h"

#include <cinttypes>

#include "util/log.h"
#include "util/time.h"

#define LOG_TAG "PacketChunkReceiver"

namespace au {

bool PacketChunkReceiver::PushPacketChunk(
    const PacketChunk::Chunk& chunk, Packet* packet) {
  if (!chunk.has_payload_id()) {
    LOGE("received packet chunk with missing payload id");
    return false;
  } else if (!chunk.has_chunk_id()) {
    LOGE("received packet chunk with missing chunk id");
    return false;
  } else if (chunk.chunk_id() == 1 && !chunk.has_total_payload_size()) {
    LOGE("received first packet chunk with missing total payload size");
    return false;
  } else if (!chunk.has_payload()) {
    LOGE("received packet chunk with missing payload");
    return false;
  }

  auto completed_it = std::find(
      completed_packets_.begin(), completed_packets_.end(),
      chunk.payload_id());
  if (completed_it != completed_packets_.end()) {
    LOGI("received packet chunk for completed payload %" PRIu32,
        chunk.payload_id());
    return false;
  }

  for (size_t i = 0; i < packets_.size(); i++) {
    auto& packet_chunks = packets_[i];
    if (packet_chunks.chunks.front().payload_id() == chunk.payload_id()) {
      // Update the last seen time for this packet id.
      packet_chunks.last_fragment_time_us = GetTimeNowUs();

      // Check that we don't already have a packet with this id.
      auto matching_chunk_it = std::find_if(
          packet_chunks.chunks.begin(), packet_chunks.chunks.end(),
          [&](const PacketChunk::Chunk& packet_chunk) {
            return packet_chunk.chunk_id() == chunk.chunk_id();
          });
      if (matching_chunk_it != packet_chunks.chunks.end()) {
        LOGI("ignoring packet chunk with id %" PRIu32
            " that has already been received", chunk.chunk_id());
        return false;
      }

      // Append the new fragment and check if we have a complete frame yet.
      packet_chunks.chunks.push_back(chunk);
      bool is_complete = packet_chunks.IsComplete(packet);
      if (is_complete) {
        completed_packets_.push_back(chunk.payload_id());
        packets_.erase(packets_.begin() + i);
      }

      return is_complete;
    }

    // TODO(aarossig): check the timestamp and remove if too old.
  }

  LOGI("receiving new payload with id %" PRIu32, chunk.payload_id());

  PacketChunks packet_chunks;
  packet_chunks.last_fragment_time_us = GetTimeNowUs();
  packet_chunks.chunks.push_back(chunk);
  if (packet_chunks.IsComplete(packet)) {
    completed_packets_.push_back(chunk.payload_id());
    return true;
  }

  packets_.push_back(packet_chunks);
  return false;
}

bool PacketChunkReceiver::PacketChunks::IsComplete(Packet* packet) {
  std::sort(chunks.begin(), chunks.end(),
      [](const PacketChunk::Chunk& a, const PacketChunk::Chunk& b) {
        return a.chunk_id() < b.chunk_id();
      });

  std::string serialized_payload;
  for (const auto& chunk : chunks) {
    serialized_payload += chunk.payload();
  }

  if (!chunks.front().has_total_payload_size()) {
    LOGI("first packet does not contain total size, id=%" PRIu32,
        chunks.front().chunk_id());
    return false;
  }

  if (serialized_payload.size() != chunks.front().total_payload_size()) {
    LOGI("packet %" PRIu32 " received %zu/%" PRIu32 " bytes",
        chunks.front().payload_id(), serialized_payload.size(),
        chunks.front().total_payload_size());
    return false;
  }

  if (!packet->ParseFromString(serialized_payload)) {
    LOGFATAL("failed to deserialize payload");
  }

  LOGI("complete packet %" PRIu32 " received %zu/%" PRIu32 " bytes",
      chunks.front().payload_id(), serialized_payload.size(),
      chunks.front().total_payload_size());
  return true;
}

}  // namespace au
