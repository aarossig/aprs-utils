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

#include "net/internet_aprs_interface.h"

#include "util/log.h"
#include "util/string.h"
#include "util/time.h"

#define LOG_TAG "InternetAPRSInterface"

namespace au {

InternetAPRSInterface::InternetAPRSInterface(
    const APRSInterface::Config& config,
    const CallsignConfig& callsign,
    const std::string& hostname, uint16_t port)
    : APRSInterface(config) {
  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, hostname.c_str(), port) < 0) {
    LOGFATAL("failed to resolve host: %s",
        SDLNet_GetError());
  }

  LOGI("connecting to %s:%" PRIu16, hostname.c_str(), port);
  socket_ = SDLNet_TCP_Open(&ip);
  if (!socket_) {
    LOGFATAL("failed to open socket: %s",
        SDLNet_GetError());
  }

  // Setup the SocketSet.
  socket_set_ = SDLNet_AllocSocketSet(1);
  if (socket_set_ == nullptr) {
    LOGFATAL("failed to init SocketSet: %s",
        SDLNet_GetError());
  }

  if (SDLNet_TCP_AddSocket(socket_set_, socket_) < 0) {
    LOGFATAL("failed to add socket: %s",
        SDLNet_GetError());
  }

  LOGI("reading server version");
  std::string server_version;
  if (!ReadServerVersion(&server_version)) {
    LOGFATAL("failed to read the server version");
  }

  LOGI("Received server version: %s", StringFormatNonPrintables(
        server_version).c_str());
  if (!Authenticate(callsign)) {
    LOGFATAL("failed to authenticate with the server");
  }
}

InternetAPRSInterface::~InternetAPRSInterface() {
  SDLNet_FreeSocketSet(socket_set_);
  SDLNet_TCP_Close(socket_);
}

bool InternetAPRSInterface::Send(const std::string& payload,
    const CallsignConfig& source, const CallsignConfig& destination,
    const std::vector<CallsignConfig>& digipeaters) {
  LOGE("sending via the internet is not supported");
  return false;
}

bool InternetAPRSInterface::Receive(
    CallsignConfig* source, CallsignConfig* destination,
    std::vector<CallsignConfig>* digipeaters, std::string* payload,
    uint32_t timeout_ms) {
  std::string packet;
  while (packet.empty()) {
    if (!ReadLine(&packet, timeout_ms)) {
      LOGE("failed to receive line");
      return false;
    } else if (StringStartsWith(packet, "#")) {
      LOGV("server sent informational packet: '%s", packet.c_str());
      packet.clear();
    }
  }

  auto separator_pos = packet.find('>');
  if (separator_pos == std::string::npos) {
    LOGE("packet missing source/destination separator: '%s'",
        StringFormatNonPrintables(packet).c_str());
    return false;
  }

  if (!source->FromString(packet.substr(0, separator_pos))) {
    LOGE("failed to parse source callsign: '%s'",
        StringFormatNonPrintables(packet).c_str());
    return false;
  }

  auto comma_pos = packet.find(',', separator_pos);
  if (comma_pos == std::string::npos) {
    LOGE("packet missing destination separator: '%s'",
        StringFormatNonPrintables(packet).c_str());
    return false;
  }

  separator_pos++;
  if (!destination->FromString(
        packet.substr(separator_pos, comma_pos - separator_pos))) {
    LOGE("failed to parse destination callsign: '%s'",
        StringFormatNonPrintables(packet).c_str());
    return false;
  }

  // TODO(aarossig): parse digipeaters.

  auto payload_pos = packet.find(':');
  if (payload_pos == std::string::npos) {
    LOGE("packet missing payload");
    return false;
  }

  *payload = packet.substr(payload_pos + 1);
  return true;
}

bool InternetAPRSInterface::ReadServerVersion(std::string* server_version) {
  const std::string kVersionPrefix = "# ";

  if (!ReadLine(server_version, /*timeout_ms=*/100)) {
    LOGE("Failed to read server version");
  } else if (!StringStartsWith(*server_version, kVersionPrefix)) {
    LOGE("Received malformed server version '%s'",
        StringFormatNonPrintables(*server_version).c_str());
  } else {
    *server_version = server_version->substr(kVersionPrefix.size(),
        server_version->size()).c_str();
    return true;
  }

  return false;
}

bool InternetAPRSInterface::Authenticate(
    const CallsignConfig& callsign) {
  const std::string auth_line = "user " + callsign.callsign + " "
    + "pass -1 "  // Authenticate with -1 as we don't send packets currently.
    + "vers watch 0.0.1";
  if (!WriteLine(auth_line)) {
    LOGE("failed to send auth line");
    return false;
  }

  std::string auth_response_line;
  if (!ReadLine(&auth_response_line, /*timeout_ms=*/100)) {
    LOGE("failed to read auth response line");
    return false;
  }

  LOGI("server responded to auth with '%s'",
      StringFormatNonPrintables(auth_response_line).c_str());
  return true;
}

bool InternetAPRSInterface::ReadLine(std::string* line, uint32_t timeout_ms) {
  constexpr size_t kMaxLineLength = 1024;

  uint64_t time_start_us = GetTimeNowUs();
  while (true) {
    if (timeout_ms > 0
        && (GetTimeNowUs() - time_start_us) > (timeout_ms * 1000)) {
      LOGE("timeout reading socket");
      return false;
    }

    int check_result = SDLNet_CheckSockets(socket_set_, /*timeout_ms=*/1);
    if (check_result < 0) {
      LOGFATAL("failed to check sockets: %s", SDLNet_GetError());
    } else if (check_result >= 1) {
      char byte;
      int read_result = SDLNet_TCP_Recv(socket_, &byte, 1);
      if (read_result <= 0) {
        LOGFATAL("failed to read from socket: %s", SDLNet_GetError());
      }

      line->push_back(byte);
      if (line->size() > kMaxLineLength) {
        LOGE("server sent line that is too long");
        return false;
      }

      if (line->size() >= 2
          && line->at(line->size() - 2) == '\r'
          && line->at(line->size() - 1) == '\n') {
        line->resize(line->size() - 2);
        return true;
      }
    }
  }
}

bool InternetAPRSInterface::WriteLine(const std::string& line) {
  std::string send_line = line + "\r\n";
  int write_result = SDLNet_TCP_Send(socket_,
      send_line.data(), send_line.size());
  if (write_result < line.size()) {
    LOGFATAL("failed to write to socket: %s", SDLNet_GetError());
  }

  return true;
}

}  // namespace au
