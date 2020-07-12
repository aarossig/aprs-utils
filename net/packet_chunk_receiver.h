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

#ifndef APRS_UTILS_NET_PACKET_CHUNK_RECEIVER_H_
#define APRS_UTILS_NET_PACKET_CHUNK_RECEIVER_H_

#include <vector>

#include "proto/packet.pb.h"
#include "util/non_copyable.h"

namespace au {

// Handles incoming packet fragments and forms complete packets.
class PacketChunkReceiver : public NonCopyable {
 public:
  // Pushes a new fragment into the fragment manager. Returns true if a
  // complete packet is received and populates the supplied packet.
  bool PushPacketChunk(const PacketChunk::Chunk& chunk, Packet* packet);

 private:
  // Incoming chunks for a given packet.
  struct PacketChunks {
    // The timestamp of the last chunk received for this packet.
    uint64_t last_fragment_time_us;

    // The chunks received for this packet thus far.
    std::vector<PacketChunk::Chunk> chunks;

    // Returns true if the chunks make a complete packet. If the packet is
    // complete, the supplied pointer is populated.
    bool IsComplete(Packet* packet);
  };

  // The list of incoming packet chunks.
  std::vector<PacketChunks> packets_;

  // The list of completed packets to avoid receiving the same frame twice.
  std::vector<uint32_t> completed_packets_;
};

}  // namespace au

#endif  // APRS_UTILS_NET_PACKET_CHUNK_RECEIVER_H_
