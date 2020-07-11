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

#ifndef APRS_UTILS_NET_APRS_INTERFACE_H_
#define APRS_UTILS_NET_APRS_INTERFACE_H_

namespace au {

// An interface to use for sending/receiving packets from a APRS.
class APRSInterface {
 public:
  virtual ~APRSInterface() = default;

  // A config for a callsign and ssid.
  struct CallsignConfig {
    std::string callsign;
    int ssid;
  };

  // Sends a frame over APRS.
  virtual bool Send(const std::string& payload,
      const CallsignConfig& source,
      const CallsignConfig& destination,
      const std::vector<CallsignConfig>& digipeaters) = 0;

  // Receives a frame from the APRS network.
  virtual bool Receive(CallsignConfig* source, CallsignConfig* destination,
      std::vector<CallsignConfig>* digipeaters, std::string* payload,
      uint32_t timeout_ms) = 0;
};

}  // namespace au

#endif  // APRS_UTILS_NET_APRS_INTERFACE_H_
