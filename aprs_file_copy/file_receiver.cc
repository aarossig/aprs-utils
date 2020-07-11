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

FileReceiver::FileReceiver(APRSInterface* aprs_interface)
    : aprs_interface_(aprs_interface) {}

bool FileReceiver::Receive(const CallsignConfig& callsign,
    const CallsignConfig& peer_callsign) {
  const CallsignConfig kBroadcastDestination({kBroadcastCallsign, 0});

  while (true) {
    CallsignConfig source;
    CallsignConfig destination;
    std::vector<CallsignConfig> digipeaters;
    std::string payload;

    if (!aprs_interface_->Receive(&source, &destination,
          &digipeaters, &payload, /*timeout_ms=*/0)) {
      continue;
    }

    if (peer_callsign.IsEmpty() && destination == kBroadcastDestination
        || peer_callsign == destination) {
      LOGI("source callsign: '%s'", source.ToString().c_str());
      LOGI("destination callsign: '%s'", destination.ToString().c_str());
      LOGI("payload: '%s'", payload.c_str());
      LOGI("");
    }

  }

  return true;
}

}  // namespace au
