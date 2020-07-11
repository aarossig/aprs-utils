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

#include "aprs_file_copy/file_receiver.h"

#include "util/log.h"
#include "util/string.h"

#define LOG_TAG "FileReceiver"

namespace au {

FileReceiver::FileReceiver(const std::string& tnc_hostname, uint16_t tnc_port)
    : tnc_connection_(tnc_hostname, tnc_port) {}

bool FileReceiver::Receive(const std::string& source_callsign) {
  while (1) {
    std::string contents;
    tnc_connection_.ReceiveFrame({"KN6FVU", 0}, 100000, &contents);
    LOGI("contents: %s %zu", StringFormatNonPrintables(contents).c_str(),
        contents.size());
    LOGI("");
  }

  return true;
}

}  // namespace au
