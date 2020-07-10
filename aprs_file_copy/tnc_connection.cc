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

namespace afc {

TNCConnection::TNCConnection(const std::string& hostname, uint16_t port) {
  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, hostname.c_str(), port) < -1) {
    LOGFATAL("FileSender: failed to resolve TNC host: %s", SDLNet_GetError());
  }

  tnc_socket_ = SDLNet_TCP_Open(&ip);
  if (!tnc_socket_) {
    LOGFATAL("FileSender: failed to open TNC socket: %s", SDLNet_GetError());
  }
}

TNCConnection::~TNCConnection() {
  SDLNet_TCP_Close(tnc_socket_);
}

bool TNCConnection::SendFrame(const std::string& information,
    const CallsignConfig& source,
    const std::vector<CallsignConfig>& digipeaters) {
  constexpr char kExperimentalCallsign = "APZ200";
  if (digipeaters.size() > 8) {
    LOGFATAL("Too many digipeaters specified");
  }

  // Encode addresses.
  std::string ax25_addresses;
  ax25_addresses += EncodeAX25Callsign({kExperimentalCallsign, 0});
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
  ax25_frame += information;

  // Format HDLC and then encapsulate in a KISS frame.
  std::string kiss_frame = EncodeKISSFrame(ax25_frame);
  if (SDLNet_TCP_Send(tnc_socket_,
        kiss_frame.data(), kiss_frame.size()) < 0) {
    LOGE("Failed to send frame: %s", SDLNet_GetError());
    return false;
  }

  return true;
}

std::string TNCConnection::EncodeAX25Callsign(
    const CallsignConfig& config, bool last) {
  if (config.ssid < 0 || config.ssid > 15) {
    LOGFATAL("Invalid SSID: %d", config.ssid);
  }

  if (config.callsign.length() > 6) {
    LOGFATAL("Invalid callsign '%s'", config.callsign);
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

}  // namespace afc
