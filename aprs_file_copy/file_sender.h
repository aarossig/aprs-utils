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

#ifndef APRS_UTILS_APRS_FILE_COPY_FILE_SENDER_H_
#define APRS_UTILS_APRS_FILE_COPY_FILE_SENDER_H_

#include <string>

#include "net/tnc_aprs_interface.h"
#include "util/non_copyable.h"

namespace au {

// An class that is responsible for sending a file over an APRS link.
class FileSender : public NonCopyable {
 public:
  // Setup the file sender with the filename to send.
  FileSender(const std::string& filename, const std::string& tnc_hostname,
      uint16_t tnc_port);

  // Sends the file to the file, returning true if successful. Status is logged.
  bool Send();

 private:
  // The contents of the file that are being sent.
  std::string file_contents_;

  // The connection to the TNC.
  TNCConnection tnc_connection_;
};

}  // namespace au

#endif  // APRS_UTILS_APRS_FILE_COPY_FILE_SENDER_H_
