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

#include <SDL_net.h>
#include <tclap/CmdLine.h>

#include "aprs_file_copy/file_sender.h"
#include "util/log.h"

// A description of the program.
constexpr char kDescription[] =
    "A file copy utility that uses APRS for backhaul.";

// The version of the program.
constexpr char kVersion[] = "0.0.1";

int main(int argc, char** argv) {
  LOGI("aprs-file-copy start");
  if (SDLNet_Init() < 0) {
    LOGFATAL("SDL_Init failed: %s", SDLNet_GetError());
  }

  // Command line flags.
  TCLAP::CmdLine cmd(kDescription, ' ', kVersion);
  TCLAP::ValueArg<std::string> send_file_arg("s", "send",
      "The file to send.", false, "", "path", cmd);
  TCLAP::SwitchArg receive_files_arg("r", "receive",
      "Set to true to receive files sent by the network.", cmd);
  TCLAP::ValueArg<std::string> tnc_hostname_arg("", "tnc_hostname",
      "The hostname of the TNC to connect to.", false, "localhost", "hostname",
      cmd);
  TCLAP::ValueArg<uint16_t> tnc_port_arg("", "tnc_port",
    "The port of the TNC to connect to.", false, 8001, "port", cmd);
  cmd.parse(argc, argv);

  int return_code = 0;
  if (!send_file_arg.getValue().empty()) {
    afc::FileSender file_sender(send_file_arg.getValue(),
        tnc_hostname_arg.getValue(), tnc_port_arg.getValue());
    if (!file_sender.Send()) {
      return_code = -1;
    }
  } else if (receive_files_arg.getValue()) {
    // TODO: setup.
  } else {
    LOGE("Must specify whether to send or receive");
    return_code = -1;
  }

  SDLNet_Quit();
  LOGI("aprs-file-copy quit");
  return return_code;
}
