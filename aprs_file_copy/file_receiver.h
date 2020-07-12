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

#include "net/aprs_interface.h"
#include "util/non_copyable.h"

namespace au {

// A class that is responsible for receiving a file from an SDR link.
class FileReceiver : public NonCopyable {
 public:
  // Setup the file receiver.
  FileReceiver(APRSInterface* aprs_interface);

  // Receives a file from the supplied callsign.
  bool Receive(const CallsignConfig& callsign,
      const CallsignConfig& peer_callsign);

 private:
  // The interface to send/receive APRS pakcets over.
  APRSInterface* const aprs_interface_;

  // Incoming chunks for a file.
  struct FileChunks {
    // The timestamp of the last update to this file chunks tracker.
    uint64_t last_time_us;

    // The header for this file transfer.
    Packet::FileTransferHeader header;

    // The set of chunks that have been received thus far.
    std::vector<Packet::FileTransferChunk> chunks;

    // Set to true when the entire file has been received.
    bool is_complete;

    // Returns the transfer id for these chunks.
    uint32_t GetId() const;
  };

  // The list of incoming file chunks.
  std::vector<FileChunks> file_chunks_;

  // Checks if a file transfer has been started for a given id.
  FileChunks* GetFileChunksForId(uint32_t id);

  // Handles a file transfer header.
  void HandleTransferHeader(const Packet::FileTransferHeader& header);

  // Handles a file transfer chunk.
  void HandleTransferChunk(const Packet::FileTransferChunk& chunk);
};

}  // namespace au

#endif  // APRS_UTILS_APRS_FILE_COPY_FILE_RECEIVER_H_
