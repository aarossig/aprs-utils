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

#ifndef APRS_UTILS_NET_INTERNET_APRS_INTERFACE_H_
#define APRS_UTILS_NET_INTERNET_APRS_INTERFACE_H_

#include <SDL_net.h>

#include "net/aprs_interface.h"
#include "util/non_copyable.h"

namespace au {

class InternetAPRSInterface : public APRSInterface,
                              public NonCopyable {
 public:
  // Setup the internet interface with hostname and port to connect to.
  InternetAPRSInterface(const APRSInterface::Config& config,
      const APRSInterface::CallsignConfig& callsign,
      const std::string& hostname, uint16_t port);

  // Close the connection.
  ~InternetAPRSInterface() override;

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
  // The socket used to interact with the server.
  TCPsocket socket_;

  // The SocketSet used to implement timeouts for receive.
  SDLNet_SocketSet socket_set_;

  // Reads the server version. This is expected to be sent on startup.
  bool ReadServerVersion(std::string* server_version);

  // Sends the authentication command.
  bool Authenticate(const APRSInterface::CallsignConfig& callsign);

  // Reads a line from the APRS-IS server. Returns true if successful and
  // populates the line.
  bool ReadLine(std::string* line, uint32_t timeout_ms);

  // Writes a line to the server. Returns true if successful.
  bool WriteLine(const std::string& line);
};

}  // namespace au

#endif  // APRS_UTILS_NET_INTERNET_APRS_INTERFACE_H_
