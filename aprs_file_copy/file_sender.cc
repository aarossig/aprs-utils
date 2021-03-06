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

#include "aprs_file_copy/file_sender.h"

#include <cstring>

#include <boost/filesystem.hpp>

#include "util/file.h"
#include "util/log.h"

#define LOG_TAG "FileSender"

namespace au {

FileSender::FileSender(APRSInterface* aprs_interface)
    : aprs_interface_(aprs_interface),
      next_transfer_id_(0) {}

bool FileSender::Send(const std::string& filename, size_t max_chunk_size,
    const CallsignConfig& callsign, const CallsignConfig& peer_callsign,
    const std::vector<CallsignConfig>& digipeaters) {
  std::string transfer_filename =
      boost::filesystem::path(filename).filename().string();
  std::string file_contents;
  if (!ReadFileToString(filename, &file_contents)) {
    LOGE("failed to read file '%s': %s (%d)",
        filename.c_str(), strerror(errno), errno);
    return false;
  }

  Packet::FileTransferHeader header;
  header.set_filename(transfer_filename);
  header.set_size(file_contents.size());
  header.set_id(GetNextTransferId());
  LOGI("sending file '%s'", filename.c_str());
  LOGI("name='%s', size=%zu", transfer_filename.c_str(), file_contents.size());

  std::vector<Packet::FileTransferChunk> chunks;
  uint32_t chunk_id = 1;
  for (size_t offset = 0; offset < file_contents.size();) {
    size_t chunk_size =
      max_chunk_size == 0 ? file_contents.size() : max_chunk_size;
    chunk_size = std::min(file_contents.size() - offset, chunk_size);

    Packet::FileTransferChunk chunk;
    chunk.set_id(header.id());
    chunk.set_chunk_id(chunk_id++);
    chunk.set_chunk(file_contents.substr(offset, chunk_size));
    chunks.push_back(chunk);
    offset += chunk_size;
  }

  bool broadcast_mode = peer_callsign.IsEmpty();
  if (broadcast_mode) {
    return SendBroadcast(header, chunks, callsign, digipeaters);
  } else {
    // TODO: implement directed mode.
    LOGE("directed mode is not supported yet");
    return false;
  }
}

bool FileSender::SendBroadcast(
    const Packet::FileTransferHeader& header,
    const std::vector<Packet::FileTransferChunk>& chunks,
    const CallsignConfig& callsign,
    const std::vector<CallsignConfig>& digipeaters) {
  Packet packet;
  *packet.mutable_file_transfer_header() = header;
  if (!aprs_interface_->SendBroadcastPacket(packet, callsign, digipeaters)) {
    LOGE("failed to send header");
    return false;
  }

  for (size_t i = 0; i < chunks.size(); i++) {
    packet.Clear();
    *packet.mutable_file_transfer_chunk() = chunks[i];
    if (!aprs_interface_->SendBroadcastPacket(
          packet, callsign, digipeaters)) {
      LOGE("failed to send chunk %zu", i);
      return false;
    }
  }

  return true;
}

uint32_t FileSender::GetNextTransferId() {
  uint32_t next_transfer_id = next_transfer_id_++;
  if (next_transfer_id == 0) {
    next_transfer_id = next_transfer_id_++;
  }

  return next_transfer_id;
}

}  // namespace au
