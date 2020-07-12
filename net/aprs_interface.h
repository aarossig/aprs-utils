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

#ifndef APRS_UTILS_NET_APRS_INTERFACE_H_
#define APRS_UTILS_NET_APRS_INTERFACE_H_

#include <string>
#include <vector>

#include "net/packet_chunk_receiver.h"
#include "proto/packet.pb.h"
#include "util/callsign.h"

namespace au {

// An interface to use for sending/receiving packets from a APRS.
class APRSInterface {
 public:
  // The configuration for this APRSInterface.
  struct Config {
    float transmit_interval_s;
    size_t max_packet_size;
  };

  // The interval in seconds between transmissions.
  static constexpr float kDefaultTransmitIntervalS = 20.0f;

  // The maximum number of bytes that can be sent at a time.
  static constexpr size_t kDefaultMaxPacketSize = 100;

  // Setup the APRSInterface.
  APRSInterface(const Config& config);

  virtual ~APRSInterface() = default;

  // Sends a packet in ACKless mode.
  bool SendBroadcastPacket(const Packet& packet,
      const CallsignConfig& source,
      const std::vector<CallsignConfig>& digipeaters);

  // Receives a packet in ACKless mode.
  bool ReceiveBroadcastPacket(Packet* packet,
      CallsignConfig* source, std::vector<CallsignConfig>* digipeaters);

  // Sends a frame over APRS. This is a lower-level interface that is not
  // typically used.
  virtual bool Send(const std::string& payload,
      const CallsignConfig& source,
      const CallsignConfig& destination,
      const std::vector<CallsignConfig>& digipeaters) = 0;

  // Receives a frame from the APRS network. This is a lower-level interface
  // that is not typically used.
  virtual bool Receive(CallsignConfig* source, CallsignConfig* destination,
      std::vector<CallsignConfig>* digipeaters, std::string* payload,
      uint32_t timeout_ms) = 0;

 private:
  // The config to use for this APRSInterface.
  const Config config_;

  // The id of the next payload from this station.
  uint32_t next_payload_id_;

  // Handles receiving chunks until completed packets are received.
  PacketChunkReceiver chunk_receiver_;

  // Returns the ID of the next payload to send.
  uint32_t GetNextPayloadId();

  // Sends a packet chunk by serializing to base64 and forming a valid APRS
  // frame.
  bool SendPacketChunk(const PacketChunk& chunk, const CallsignConfig& source,
      const CallsignConfig& destination,
      const std::vector<CallsignConfig>& digipeaters);
};

}  // namespace au

#endif  // APRS_UTILS_NET_APRS_INTERFACE_H_
