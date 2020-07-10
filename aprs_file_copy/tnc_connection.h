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

#ifndef APRS_FILE_COPY_TNC_CONNECTION_H_
#define APRS_FILE_COPY_TNC_CONNECTION_H_

#include <string>
#include <vector>

#include <SDL_net.h>

#include "util/non_copyable.h"

namespace afc {

// A connection to a TNC for sending/receiving frames.
class TNCConnection : public NonCopyable {
 public:
  // Setup the connection to the TNC.
  TNCConnection(const std::string& hostname, uint16_t port);

  // Close the connection.
  ~TNCConnection();

  // A config for a callsign and ssid.
  struct CallsignConfig {
    std::string callsign;
    int ssid;
  };

  // Sends a frame to the TNC.
  bool SendFrame(const std::string& information,
      const CallsignConfig& source,
      const std::vector<CallsignConfig>& digipeaters);

 private:
  // The TCP socket used to communicate with the terminal node controller (TNC).
  TCPsocket tnc_socket_;

  // Encodes an AX.25 formatted callsign.
  std::string EncodeAX25Callsign(const CallsignConfig& config,
      bool last = false);

  // Encodes a KISS TNC frame.
  std::string EncodeKISSFrame(const std::string& hdlc_frame);
};

}  // namespace afc

#endif  // APRS_FILE_COPY_TNC_CONNECTION_H_
