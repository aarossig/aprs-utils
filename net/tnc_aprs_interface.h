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

#ifndef APRS_UTILS_NET_TNC_APRS_INTERFACE_H_
#define APRS_UTILS_NET_TNC_APRS_INTERFACE_H_

#include <string>
#include <vector>

#include <SDL_net.h>

#include "net/aprs_interface.h"
#include "util/non_copyable.h"

namespace au {

// A connection to a TNC for sending/receiving frames.
class TNCConnection : public APRSInterface,
                      public NonCopyable {
 public:
  // Setup the connection to the TNC.
  TNCConnection(const std::string& hostname, uint16_t port);

  // Close the connection.
  ~TNCConnection();

 protected:
  // APRSInterface implementation.
  bool Send(const std::string& payload,
      const CallsignConfig& source,
      const CallsignConfig& destination,
      const std::vector<CallsignConfig>& digipeaters) final;
  bool Receive(CallsignConfig* source, CallsignConfig* destination,
      std::vector<CallsignConfig>* digipeaters, std::string* payload,
      uint32_t timeout_ms) final;

 private:
  // The TCP socket used to communicate with the terminal node controller (TNC).
  TCPsocket tnc_socket_;

  // The SocketSet used to implement timeouts for receive.
  SDLNet_SocketSet socket_set_;

  // Encodes an AX.25 formatted callsign.
  std::string EncodeAX25Callsign(const CallsignConfig& config,
      bool last = false);

  // Decodes an AX25 callsign from the supplied string at the supplied offset.
  // Returns the location of the next callsign if one is found. If this returns
  // 0, then there was a failure to decode and the frame may be invalid.
  size_t DecodeAX25Callsign(const std::string& frame, size_t offset,
      CallsignConfig* config, bool* last);

  // Encodes a KISS TNC frame.
  std::string EncodeKISSFrame(const std::string& hdlc_frame);

  // Decodes a KISS TNC frame, or returns an empty string if there is a
  // timeout.
  std::string DecodeKISSFrame(uint32_t timeout_ms);
};

}  // namespace au

#endif  // APRS_UTILS_NET_TNC_APRS_INTERFACE_H_
