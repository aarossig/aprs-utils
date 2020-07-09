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

#include "aprs_file_copy/file_sender.h"

#include <cstring>

#include "util/file.h"
#include "util/log.h"
#include "util/string.h"

#include "unistd.h"

namespace afc {

FileSender::FileSender(const std::string& filename,
    const std::string& tnc_hostname, uint16_t tnc_port) {
  if (!ReadFileToString(filename, &file_contents_)) {
    LOGFATAL("FileSender: failed to read file: %s (%d)",
        strerror(errno), errno);
  }

  IPaddress ip;
  if (SDLNet_ResolveHost(&ip, tnc_hostname.c_str(), tnc_port) < -1) {
    LOGFATAL("FileSender: failed to resolve TNC host: %s", SDLNet_GetError());
  }

  tnc_socket_ = SDLNet_TCP_Open(&ip);
  if (!tnc_socket_) {
    LOGFATAL("FileSender: failed to open TNC socket: %s", SDLNet_GetError());
  }

  LOGI("Ready to send file '%s'", filename.c_str());
}

FileSender::~FileSender() {
  SDLNet_TCP_Close(tnc_socket_);
}

bool FileSender::Send() {
  auto packet = StringFormat(
      "\xc0%cKN6FVU>APX216,WIDE2-2:=3724.69N/12150.80Wx\xc0", '\0');
  LOGI("packet size %zu", packet.size());
  SDLNet_TCP_Send(tnc_socket_, packet.data(), packet.length());
  usleep(1000000);

  return true;
}

}  // namespace afc
