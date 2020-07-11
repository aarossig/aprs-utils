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

#include "aprs_file_copy/tnc_connection.h"

#include "util/log.h"
#include "util/time.h"

#define LOG_TAG "TNCConnection"

namespace au {
namespace {

// The callsign of this app.
constexpr char kAppCallsign[] = "APZ200";

}  // namespace

TNCConnection::TNCConnection(const std::string& hostname, uint16_t port) {
  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, hostname.c_str(), port) < 0) {
    LOGFATAL("failed to resolve TNC host: %s",
        SDLNet_GetError());
  }

  tnc_socket_ = SDLNet_TCP_Open(&ip);
  if (!tnc_socket_) {
    LOGFATAL("failed to open TNC socket: %s",
        SDLNet_GetError());
  }

  // Setup the SocketSet.
  socket_set_ = SDLNet_AllocSocketSet(1);
  if (socket_set_ == nullptr) {
    LOGFATAL("failed to init SocketSet: %s",
        SDLNet_GetError());
  }

  if (SDLNet_TCP_AddSocket(socket_set_, tnc_socket_) < 0) {
    LOGFATAL("failed to add socket: %s",
        SDLNet_GetError());
  }
}

TNCConnection::~TNCConnection() {
  SDLNet_TCP_Close(tnc_socket_);
  SDLNet_FreeSocketSet(socket_set_);
}

bool TNCConnection::SendFrame(const std::string& payload,
    const CallsignConfig& source,
    const std::vector<CallsignConfig>& digipeaters) {
  if (digipeaters.size() > 8) {
    LOGFATAL("too many digipeaters specified");
  }

  // Encode addresses.
  std::string ax25_addresses;
  ax25_addresses += EncodeAX25Callsign({kAppCallsign, 0});
  ax25_addresses += EncodeAX25Callsign(source, /*last=*/digipeaters.empty());
  for (size_t i = 0; i < digipeaters.size(); i++) {
    ax25_addresses += EncodeAX25Callsign(digipeaters[i],
        /*last=*/(i == (digipeaters.size() - 1)));
  }

  // Build frame body.
  std::string ax25_frame;
  ax25_frame += ax25_addresses;
  ax25_frame += "\x03";     // UI-Frame.
  ax25_frame += "\xf0";     // No layer 3 protocol.
  ax25_frame += payload;

  // Format HDLC and then encapsulate in a KISS frame.
  std::string kiss_frame = EncodeKISSFrame(ax25_frame);
  if (SDLNet_TCP_Send(tnc_socket_,
        kiss_frame.data(), kiss_frame.size()) < 0) {
    LOGE("failed to send frame: %s", SDLNet_GetError());
    return false;
  }

  return true;
}

bool TNCConnection::ReceiveFrame(const CallsignConfig& source,
    uint32_t timeout_ms, std::string* payload) {
  std::string frame = DecodeKISSFrame(timeout_ms);
  if (frame.empty()) {
    return false;
  }

  size_t offset = 0;
  bool last = false;
  CallsignConfig destination;
  offset = DecodeAX25Callsign(frame, offset, &destination, &last);
  if (offset == 0) {
    return false;
  }

  CallsignConfig received_source;
  offset = DecodeAX25Callsign(frame, offset, &received_source, &last);
  if (offset == 0) {
    return false;
  }

  LOGI("destination %s-%d",
      destination.callsign.c_str(), destination.ssid);
  LOGI("source %s-%d",
      received_source.callsign.c_str(), received_source.ssid);

  for (size_t i = 0; i < 8 && !last; i++) {
    CallsignConfig digipeater;
    offset = DecodeAX25Callsign(frame, offset, &digipeater, &last);
    if (offset == 0) {
      return false;
    }

    LOGI("digipeater %zu %s-%d", i, digipeater.callsign.c_str(),
        digipeater.ssid);

    if (i == 7 && last) {
      LOGE("too many digipeaters");
      return false;
    }
  }

  if (static_cast<uint8_t>(frame[offset]) != 0x03) {
    LOGE("invalid frame type: 0x%02x",
        static_cast<uint8_t>(frame[offset]));
    return false;
  }

  offset++;
  if (static_cast<uint8_t>(frame[offset]) != 0xf0) {
    LOGE("invalid layer 3 protocol: 0x%02x",
        static_cast<uint8_t>(frame[offset]));
    return false;
  }

  offset++;
  *payload = frame.substr(offset);
  return true;
}

std::string TNCConnection::EncodeAX25Callsign(
    const CallsignConfig& config, bool last) {
  if (config.ssid < 0 || config.ssid > 15) {
    LOGFATAL("invalid SSID: %d", config.ssid);
  }

  if (config.callsign.length() > 6) {
    LOGFATAL("invalid callsign '%s'", config.callsign);
  }

  std::string padded_callsign = config.callsign;
  padded_callsign.resize(6, ' ');

  std::string formatted_callsign;
  for (size_t i = 0; i < padded_callsign.size(); i++) {
    formatted_callsign += (padded_callsign[i] << 1);
  }

  return formatted_callsign + static_cast<char>(
      0x60 | (config.ssid << 1) | (last ? 0x01 : 0x00));
}

size_t TNCConnection::DecodeAX25Callsign(
    const std::string& frame, size_t offset,
    CallsignConfig* config, bool* last) {
  if ((offset + 7) > frame.size()) {
    LOGE("unable to decode callsign with short frame");
    return 0;
  }

  // Check that the SSID mask matches.
  if ((frame[offset + 6] & 0x60) != 0x60) {
    LOGE("unable to decode callsign with SSID mask");
    return 0;
  }

  config->callsign.clear();
  for (size_t i = offset; i < (offset + 6); i++) {
    char c = (static_cast<uint8_t>(frame[i]) >> 1);
    if (c == ' ') {
      break;
    } else {
      config->callsign += c;
    }
  }

  *last = (frame[offset + 6] & 0x01) > 0;
  config->ssid = (static_cast<uint8_t>(frame[offset + 6] & 0x17) >> 1);
  return offset + 7;
}

std::string TNCConnection::EncodeKISSFrame(const std::string& hdlc_frame) {
  std::string kiss_frame = "\xc0";
  kiss_frame += '\0';  // Channel. TODO: Make configurable.
  for (const auto& byte : hdlc_frame) {
    if (byte == 0xc0) {
      kiss_frame += "\xdb\xdc";
    } else if (byte == 0xdb) {
      kiss_frame += "\xdb\xdd";
    } else {
      kiss_frame += byte;
    }
  }

  kiss_frame += "\xc0";
  return kiss_frame;
}

std::string TNCConnection::DecodeKISSFrame(uint32_t timeout_ms) {
  std::string frame;
  bool in_frame = false;
  bool in_escape = false;
  bool next_byte_is_header = false;
  uint64_t time_start_us = GetTimeNowUs();
  while (true) {
    if (timeout_ms != 0 &&
        (GetTimeNowUs() - time_start_us) > timeout_ms * 1000) {
      LOGE("timeout reading packet");
      return std::string();
    }

    int check_result = SDLNet_CheckSockets(socket_set_, /*timeout_ms=*/1);
    if (check_result < 0) {
      LOGFATAL("failed to check sockets: %s", SDLNet_GetError());
    } else if (check_result >= 1) {
      uint8_t byte;
      int read_result = SDLNet_TCP_Recv(tnc_socket_, &byte, 1);
      if (read_result <= 0) {
        LOGFATAL("failed to read from TNC socket: %s", SDLNet_GetError());
      } else if (byte == 0xc0) {
        if (!frame.empty()) {
          return frame;
        }

        next_byte_is_header = true;
      } else if (next_byte_is_header) {
        if ((byte & 0x0f) == 0) {
          in_frame = true;
        } else {
          LOGE("invalid KISS command: %02x", byte);
        }

        next_byte_is_header = false;
      } else if (in_frame) {
        if (byte == 0xdb) {
          if (in_escape) {
            LOGE("invalid escape sequence");
            in_escape = false;
            frame.clear();
          } else {
            in_escape = true;
          }
        } else if (in_escape) {
          in_escape = false;
          if (byte == 0xdc) {
            frame += "\xc0";
          } else if (byte == 0xdd) {
            frame += "\xdb";
          } else {
            LOGE("invalid escape sequence");
            in_frame = false;
            frame.clear();
          }
        } else {
          frame += byte;
        }
      } else {
        LOGE("KISS byte received out of frame");
      }
    }
  }
}

}  // namespace au
