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

#include "net/aprs_interface.h"
#include "util/non_copyable.h"

namespace au {

// An class that is responsible for sending a file over an APRS link.
class FileSender : public NonCopyable {
 public:
  // Setup the file sender with the filename to send.
  FileSender(APRSInterface* aprs_interface);

  // Sends the file to the file, returning true if successful. Status is logged.
  bool Send(const std::string& filename, size_t max_chunk_size,
      const APRSInterface::CallsignConfig& callsign,
      const APRSInterface::CallsignConfig& peer_callsign,
      const std::vector<APRSInterface::CallsignConfig>& digipeaters);

 private:
  // The interface to send/receive APRS packets over.
  APRSInterface* const aprs_interface_;

  // The next transfer ID to use when sending a file.
  uint32_t next_transfer_id_;

  // Broadcasts a file (ACKless mode). This will work with peers that are in
  // either RF range or if the sender is within range of an I-Gate and the
  // receiver listens there.
  bool SendBroadcast(const Packet::FileTransferHeader& header,
      const std::vector<Packet::FileTransferChunk>& chunks,
      const APRSInterface::CallsignConfig& callsign,
      const std::vector<APRSInterface::CallsignConfig>& digipeaters);

  // Returns the next transfer id.
  uint32_t GetNextTransferId();
};

}  // namespace au

#endif  // APRS_UTILS_APRS_FILE_COPY_FILE_SENDER_H_
