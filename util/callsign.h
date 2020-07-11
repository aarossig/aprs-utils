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

#ifndef APRS_UTILS_UTIL_CALLSIGN_H_
#define APRS_UTILS_UTIL_CALLSIGN_H_

#include <string>

// Utils for handling callsigns.

namespace au {

extern const char* kBroadcastCallsign;

// A config for a callsign and ssid.
struct CallsignConfig {
  std::string callsign;
  int ssid = 0;

  // Attempts to parse a callsign into this config from a string.
  bool FromString(const std::string& str);

  // Formats this callsign into a string.
  std::string ToString() const;

  // Returns true if the callsign config is empty.
  bool IsEmpty() const { return callsign.empty(); }

  // Return true if two callsign configs are equal.
  bool operator==(const CallsignConfig& other) const {
    return callsign == other.callsign && ssid == other.ssid;
  }
};

}  // namespace au

#endif  // APRS_UTILS_UTIL_CALLSIGN_H_
