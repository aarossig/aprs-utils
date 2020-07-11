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

#ifndef APRS_UTILS_APRS_FILE_COPY_FILE_RECEIVER_H_
#define APRS_UTILS_APRS_FILE_COPY_FILE_RECEIVER_H_

#include <string>

#include "net/tnc_aprs_interface.h"
#include "util/non_copyable.h"

namespace au {

// A class that is responsible for receiving a file from an SDR link.
class FileReceiver : public NonCopyable {
 public:
  // Setup the file receiver.
  FileReceiver(const std::string& tnc_hostname, uint16_t tnc_port);

  // Receives a file from the supplied callsign.
  bool Receive(const std::string& source_callsign);

 private:
  // The connection to the TNC.
  TNCConnection tnc_connection_;
};

}  // namespace au

#endif  // APRS_UTILS_APRS_FILE_COPY_FILE_RECEIVER_H_
