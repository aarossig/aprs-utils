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

#include "util/string.h"

#include <cctype>
#include <cstdarg>
#include <regex>

extern "C" {

#include <b64/cdecode.h>
#include <b64/cencode.h>

}  // extern "C"

#include "util/log.h"

namespace au {

bool StringStartsWith(const std::string& string, const std::string& prefix) {
  return string.substr(0, prefix.size()) == prefix;
}

std::vector<std::string> StringSplit(const std::string& str,
    const std::string& delimiter) {
  std::string str_copy = str;;
  std::vector<std::string> result;
  size_t pos;
  while ((pos = str_copy.find(delimiter)) != std::string::npos) {
    result.push_back(str_copy.substr(0, pos));
    str_copy.erase(0, pos + delimiter.length());
  }

  return result;
}

std::string StringReplace(const std::string& str,
    const std::string& from, const std::string& to) {
  return std::regex_replace(str, std::regex(from), to);
}

std::string StringFormat(const char* format, ...) {
  va_list vl, vl_copy;
  va_start(vl, format);
  va_copy(vl_copy, vl);

  int size = vsnprintf(nullptr, 0, format, vl);
  if (size < 0) {
    LOGFATAL("StringFormat: failed to determine output size");
  }

  std::string output = std::string(size + 1, '\0');
  size = vsnprintf(output.data(), output.size(), format, vl_copy);
  if (size < 0) {
    LOGFATAL("StringFormat: failed to format output");
  }

  va_end(vl_copy);
  va_end(vl);
  return output;
}

std::string StringBase64Encode(const std::string& str) {
  // Allocate more than enough space for the base64 library to work in.
  std::string base64(str.size() * 2, '\0');

  // This library is only mildly terrifying in that it does not accept the
  // length of the output buffer as an input (you know... snprintf vs sprintf).
  // We will just hope and pray that this is safe. A manual inspection of the
  // current implementation says it will be, but that could change.
  base64_encodestate encodestate;
  base64_init_encodestate(&encodestate);
  size_t size = base64_encode_block(str.data(), str.size(),
      base64.data(), &encodestate);
  size += base64_encode_blockend(&base64.data()[size], &encodestate);
  base64.resize(size);

  // Also, wtf it adds newlines inexplicibly every 72 characters, so let's
  // remove those. Sad.
  return StringReplace(base64, "\n", "");
}

std::string StringBase64Decode(const std::string& str) {
  std::string output(str.size(), '\0');

  base64_decodestate decodestate;
  base64_init_decodestate(&decodestate);
  int size = base64_decode_block(str.c_str(), str.size(), output.data(),
      &decodestate);
  output.resize(size);
  return output;
}

std::string StringFormatNonPrintables(const std::string& str) {
  std::string printable;
  for (size_t i = 0; i < str.size(); i++) {
    char c = str[i];
    if (isprint(c)) {
      printable += c;
    } else {
      printable += StringFormat("<0x%02x>", static_cast<uint8_t>(c));
    }
  }

  return printable;
}

}  // namespace au
