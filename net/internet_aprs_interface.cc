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
#include "util/time.h"

#define LOG_TAG "InternetAPRSInterface"

namespace au {

InternetAPRSInterface::InternetAPRSInterface(
    const APRSInterface::Config& config,
    const std::string& hostname, uint16_t port)
    : APRSInterface(config) {
  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, hostname.c_str(), port) < 0) {
    LOGFATAL("failed to resolve TNC host: %s",
        SDLNet_GetError());
  }

  socket_ = SDLNet_TCP_Open(&ip);
  if (!socket_) {
    LOGFATAL("failed to open TNC socket: %s",
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
}

InternetAPRSInterface::~InternetAPRSInterface() {
  SDLNet_FreeSocketSet(socket_set_);
  SDLNet_TCP_Close(socket_);
}

bool InternetAPRSInterface::Send(const std::string& payload,
    const CallsignConfig& source,
    const CallsignConfig& destination,
    const std::vector<CallsignConfig>& digipeaters) {
  LOGE("sending via the internet is not supported");
  return false;
}

bool InternetAPRSInterface::Receive(
    CallsignConfig* source, CallsignConfig* destination,
    std::vector<CallsignConfig>* digipeaters, std::string* payload,
    uint32_t timeout_ms) {
  return true;
}

bool InternetAPRSInterface::ReadLine(std::string* line, uint32_t timeout_ms) {
  constexpr size_t kMaxLineLength = 1024;
  line->resize(kMaxLineLength, '\0');

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
      if (line->size() >= 2
          && line->at(line->size() - 2) == '\r'
          && line->at(line->size() - 1) == '\n') {
        line->resize(line->size() - 2);
        return true;
      }
    }
  }
}

}  // namespace au
