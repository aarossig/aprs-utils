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

#include "aprs_file_copy/file_receiver.h"

#include <cinttypes>

#include "util/file.h"
#include "util/log.h"
#include "util/string.h"
#include "util/time.h"

#define LOG_TAG "FileReceiver"

namespace au {

FileReceiver::FileReceiver(APRSInterface* aprs_interface)
    : aprs_interface_(aprs_interface) {}

bool FileReceiver::Receive(const CallsignConfig& callsign,
    const CallsignConfig& peer_callsign) {
  const CallsignConfig kBroadcastDestination({kBroadcastCallsign, 0});

  while (true) {
    Packet packet;
    CallsignConfig source;
    std::vector<CallsignConfig> digipeaters;
    if (!aprs_interface_->ReceiveBroadcastPacket(
          &packet, &source, &digipeaters)) {
      continue;
    }

    if (packet.has_file_transfer_header()) {
      LOGI("received transfer request with id %" PRIu32 " for file '%s'",
          packet.file_transfer_header().id(),
          StringFormatNonPrintables(
            packet.file_transfer_header().filename()).c_str());
    } else if (packet.has_file_transfer_chunk()) {
      LOGI("received transfer chunk id %" PRIu32 " for transfer %" PRIu32,
          packet.file_transfer_chunk().chunk_id(),
          packet.file_transfer_chunk().id());
    }

    switch (packet.type_case()) {
      case Packet::kFileTransferHeader:
        HandleTransferHeader(packet.file_transfer_header());
        break;
      case Packet::kFileTransferChunk:
        HandleTransferChunk(packet.file_transfer_chunk());
        break;
      default:
        LOGE("invalid packet received");
    }
  }

  return true;
}

uint32_t FileReceiver::FileChunks::GetId() const {
  if (header.has_id()) {
    return header.id();
  }

  if (chunks.empty() || !chunks.front().has_id()) {
    LOGFATAL("invalid FileChunks tracker");
  }

  return chunks.front().id();
}

FileReceiver::FileChunks* FileReceiver::GetFileChunksForId(uint32_t id) {
  for (auto& file_chunks : file_chunks_) {
    if (file_chunks.GetId() == id) {
      // TODO: handle old chunks.
      return &file_chunks;
    }
  }

  return nullptr;
}

void FileReceiver::HandleTransferHeader(
    const Packet::FileTransferHeader& header) {
  if (!header.has_id()) {
    LOGE("received header with missing id");
    return;
  } else if (!header.has_size()) {
    LOGE("received header with missing size");
    return;
  } else if (!header.has_filename()) {
    LOGE("received header with missing filename");
    return;
  }

  auto file_chunks = GetFileChunksForId(header.id());
  if (file_chunks == nullptr) {
    FileChunks chunks;
    chunks.last_time_us = GetTimeNowUs();
    chunks.header = header;
    file_chunks_.push_back(chunks);
  } else {
    file_chunks->last_time_us = GetTimeNowUs();
    file_chunks->header = header;
  }
}

void FileReceiver::HandleTransferChunk(
  const Packet::FileTransferChunk& chunk) {
  if (!chunk.has_id()) {
    LOGE("received chunk with missing id");
    return;
  } else if (!chunk.has_chunk_id()) {
    LOGE("received chunk with missing chunk id");
    return;
  } else if (!chunk.has_chunk()) {
    LOGE("received chunk with no contents");
    return;
  }

  auto file_chunks = GetFileChunksForId(chunk.id());
  if (file_chunks == nullptr) {
    FileChunks chunks;
    chunks.last_time_us = GetTimeNowUs();
    chunks.chunks.push_back(chunk);
    file_chunks_.push_back(chunks);
  } else {
    file_chunks->last_time_us = GetTimeNowUs();

    uint32_t chunk_id = 1;
    for (size_t i = 0; i < file_chunks->chunks.size(); i++) {
      if (file_chunks->chunks[i].chunk_id() != chunk_id) {
        LOGE("transfer %" PRIu32 " is missing chunk %" PRIu32,
            chunk.id(), chunk_id);
        break;
      }
    }

    for (const auto& existing_chunk : file_chunks->chunks) {
      if (existing_chunk.chunk_id() == chunk.chunk_id()) {
        if (file_chunks->header.has_filename()) {
          LOGI("ignoring chunk id %" PRIu32 " that '%s' has already received",
              existing_chunk.chunk_id(),
              file_chunks->header.filename().c_str());
        } else {
          LOGI("ignoring chunk id %" PRIu32 " that transfer %" PRIu32 " has "
              "already received", existing_chunk.chunk_id(),
              file_chunks->GetId());
        }

        return;
      }
    }

    file_chunks->chunks.push_back(chunk);
    std::sort(file_chunks->chunks.begin(), file_chunks->chunks.end(),
        [](const Packet::FileTransferChunk& a,
           const Packet::FileTransferChunk& b) {
          return a.chunk_id() < b.chunk_id();
        });

    chunk_id = 1;
    std::string file_contents;
    for (const auto& chunk : file_chunks->chunks) {
      if (chunk.chunk_id() != chunk_id++) {
        break;
      }

      file_contents += chunk.chunk();
    }

    if (!file_contents.empty() && file_chunks->header.filename().empty()) {
      LOGI("header unavailable to write file contents");
    } else if (!file_contents.empty()) {
      // TODO(aarossig): sanitize the path.
      LOGI("writing file '%s' to disk", file_chunks->header.filename().c_str());
      WriteStringToFile(file_chunks->header.filename(),
          file_contents);
    }

    if (file_contents.size() == file_chunks->header.size()) {
      LOGI("file transfer '%s' complete",
          file_chunks->header.filename().c_str());
    }
  }
}

}  // namespace au
