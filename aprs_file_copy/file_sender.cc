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

#include "unistd.h"

#define LOG_TAG "FileSender"

namespace au {

FileSender::FileSender(const std::string& filename,
    APRSInterface* aprs_interface)
    : aprs_interface_(aprs_interface) {
  if (!ReadFileToString(filename, &file_contents_)) {
    LOGFATAL("failed to read file: %s (%d)",
        strerror(errno), errno);
  }

  LOGI("ready to send file '%s'", filename.c_str());
}

bool FileSender::Send(const std::string& callsign,
    const std::string& peer_callsign) {
  return true;
}

}  // namespace au
