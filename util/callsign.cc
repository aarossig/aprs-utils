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

#include "util/callsign.h"

#include "util/string.h"

namespace au {

// TODO(aarossig): This is currently an experimental callsign. Consider
// requesting another if this tool gains traction.
const char* kBroadcastCallsign = "APZ222";

bool CallsignConfig::FromString(const std::string& str) {
  auto dash_pos = str.find('-');
  if (dash_pos == std::string::npos) {
    callsign = str;
  } else {
    callsign = str.substr(0, dash_pos);
    // TODO(aarossig): parse the SSID.
  }

  return true;
}

std::string CallsignConfig::ToString() const {
  if (ssid > 0) {
    return StringFormat("%s-%d", callsign.c_str(), ssid);
  } else {
    return callsign;
  }
}

}  // namespace au
