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
  TCLAP::ValueArg<std::string> tnc_hostname_arg("", "tnc_hostname",
      "The hostname of the TNC to connect to.", false, "localhost",
      "hostname", cmd);
  TCLAP::ValueArg<uint16_t> tnc_port_arg("", "tnc_port",
    "The port of the TNC to connect to.", false, 8001,
    "port", cmd);
  TCLAP::ValueArg<std::string> aprs_is_hostname_arg("", "aprs_is_hostname",
      "The hostname of the APRS-IS service to connect to.", false,
      "rotate.aprs2.net", "hostname", cmd);
  TCLAP::ValueArg<uint16_t> aprs_is_port_arg("", "aprs_is_port",
      "The port of the APRS-IS service to connect to.", false, 14580,
      "port", cmd);
  cmd.parse(argc, argv);

  // Validate arguments.
  if (!send_file_arg.getValue().empty() && use_aprs_is_arg.getValue()) {
    LOGFATAL("unable to use APRS-IS to send files");
  }

  // Setup the APRS interface.
  std::unique_ptr<au::APRSInterface> aprs_interface;
  if (use_aprs_is_arg.getValue()) {
    aprs_interface = std::make_unique<au::InternetAPRSInterface>(
        aprs_is_hostname_arg.getValue(), aprs_is_port_arg.getValue());
  } else {
    aprs_interface = std::make_unique<au::TNCAPRSInterface>(
        tnc_hostname_arg.getValue(), tnc_port_arg.getValue());
  }

  // Perform the file transger operation.
  int return_code = -1;
  if (!send_file_arg.getValue().empty()) {
    au::FileSender file_sender(send_file_arg.getValue(), aprs_interface.get());
    if (file_sender.Send(callsign_arg.getValue(),
          peer_callsign_arg.getValue())) {
      return_code = 0;
    }
  } else if (receive_arg.getValue()) {
    au::FileReceiver file_receiver(aprs_interface.get());
    if (file_receiver.Receive(callsign_arg.getValue(),
          peer_callsign_arg.getValue())) {
      return_code = 0;
    }
  } else {
    LOGFATAL("must specify whether to send or receive");
  }

  SDLNet_Quit();
  LOGI("success");
  return return_code;
}
