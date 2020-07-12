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

#include <tclap/CmdLine.h>

#include "aprs_file_copy/file_sender.h"
#include "aprs_file_copy/file_receiver.h"
#include "net/internet_aprs_interface.h"
#include "net/tnc_aprs_interface.h"
#include "util/log.h"

#define LOG_TAG "APRSFileCopy"

// A description of the program.
constexpr char kDescription[] =
    "A file copy utility that uses APRS for backhaul.";

// The version of the program.
constexpr char kVersion[] = "0.0.1";

int main(int argc, char** argv) {
  // Init.
  LOGI("start");
  if (SDLNet_Init() < 0) {
    LOGFATAL("SDL_Init failed: %s", SDLNet_GetError());
  }

  // Command line flags.
  TCLAP::CmdLine cmd(kDescription, ' ', kVersion);
  TCLAP::ValueArg<std::string> callsign_arg("c", "callsign",
      "Set to the callsign of this station.", true, "", "callsign", cmd);
  TCLAP::ValueArg<std::string> peer_callsign_arg("p",
      "peer_callsign", "Set to the callsign of the other station. "
      "If this is left empty, files are sent to all stations (no ACKs) and "
      "all files are received (broadcast mode).", false, "", "callsign", cmd);
  TCLAP::ValueArg<std::string> send_file_arg("s", "send",
      "The file to send.", false, "", "path", cmd);
  TCLAP::SwitchArg receive_arg("r", "receive",
      "Set to true to receive files sent by the network.", cmd);
  TCLAP::SwitchArg use_aprs_is_arg("", "use_aprs_is",
      "Set to true to use the APRS-IS network to receive files.", cmd);
  TCLAP::ValueArg<size_t> max_file_chunk_size_arg ("", "max_file_chunk_size",
      "The size of a file chunk to transfer. Handy for formats "
      "that support progressive encoding such as JPEG or text files. Passing "
      "zero will not chunk the file.",
      false, 0, "bytes", cmd);
  TCLAP::ValueArg<float> aprs_transmit_interval_s_arg("",
      "aprs_transmit_interval_s",
      "The amount of time between APRS transmissions.",
      false, au::APRSInterface::kDefaultTransmitIntervalS, "seconds", cmd);
  TCLAP::ValueArg<size_t> aprs_max_packet_size_arg("", "aprs_max_packet_size",
      "The maximum size of an APRS packet to transfer.",
      false, au::APRSInterface::kDefaultMaxPacketSize, "bytes", cmd);
  TCLAP::ValueArg<size_t> aprs_retransmit_count_arg("",
      "aprs_retransmit_count", "The number of times to retransmit a packet.",
      false, au::APRSInterface::kDefaultRetransmitCount, "count", cmd);
  TCLAP::ValueArg<std::string> tnc_hostname_arg("", "tnc_hostname",
      "The hostname of the TNC to connect to.", false, "localhost",
      "hostname", cmd);
  TCLAP::ValueArg<uint16_t> tnc_port_arg("", "tnc_port",
    "The port of the TNC to connect to.", false, 8001,
    "port", cmd);
  TCLAP::ValueArg<std::string> aprs_is_hostname_arg("", "aprs_is_hostname",
      "The hostname of the APRS-IS service to connect to.", false,
      "rotate.aprs.net", "hostname", cmd);
  TCLAP::ValueArg<uint16_t> aprs_is_port_arg("", "aprs_is_port",
      "The port of the APRS-IS service to connect to.", false, 10152,
      "port", cmd);
  cmd.parse(argc, argv);

  // Validate arguments.
  if (!send_file_arg.getValue().empty() && use_aprs_is_arg.getValue()) {
    LOGFATAL("unable to use APRS-IS to send files");
  }

  // TODO: parse all callsign arguments into CallsignConfig.

  au::APRSInterface::Config aprs_config;
  aprs_config.transmit_interval_s = aprs_transmit_interval_s_arg.getValue();
  aprs_config.retransmit_count = aprs_retransmit_count_arg.getValue();
  aprs_config.max_packet_size = aprs_max_packet_size_arg.getValue();

  // Setup the APRS interface.
  std::unique_ptr<au::APRSInterface> aprs_interface;
  if (use_aprs_is_arg.getValue()) {
    aprs_interface = std::make_unique<au::InternetAPRSInterface>(
        aprs_config, au::CallsignConfig({callsign_arg.getValue(), 0}),
        aprs_is_hostname_arg.getValue(), aprs_is_port_arg.getValue());
  } else {
    aprs_interface = std::make_unique<au::TNCAPRSInterface>(
        aprs_config, tnc_hostname_arg.getValue(), tnc_port_arg.getValue());
  }

  // Perform the file transger operation.
  int return_code = -1;
  if (!send_file_arg.getValue().empty()) {
    au::FileSender file_sender(aprs_interface.get());
    if (file_sender.Send(send_file_arg.getValue(),
          max_file_chunk_size_arg.getValue(), {callsign_arg.getValue(), 0},
          {peer_callsign_arg.getValue(), 0}, {})) {
      return_code = 0;
    }
  } else if (receive_arg.getValue()) {
    au::FileReceiver file_receiver(aprs_interface.get());
    if (file_receiver.Receive({callsign_arg.getValue(), 0},
          {peer_callsign_arg.getValue(), 0})) {
      return_code = 0;
    }
  } else {
    LOGFATAL("must specify whether to send or receive");
  }

  SDLNet_Quit();
  LOGI("success");
  return return_code;
}
