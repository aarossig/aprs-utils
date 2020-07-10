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

#ifndef APRS_UTILS_UTIL_STRING_H_
#define APRS_UTILS_UTIL_STRING_H_

#include <string>
#include <vector>

namespace au {

// Returns true if the supplied string starts with another.
bool StringStartsWith(const std::string& string, const std::string& prefix);

// Splits a string into pieces.
std::vector<std::string> StringSplit(const std::string& str,
    const std::string& delimiter);

// Find and replace all instances of from and replace with to in the supplied
// string.
std::string StringReplace(const std::string& str,
    const std::string& from, const std::string& to);

// Formats the supplied arguments into a string and returns it.
std::string StringFormat(const char* format, ...);

// Encodes the supplied string with base64.
std::string StringBase64Encode(const std::string& str);

// Decodes the supplied string from base64.
std::string StringBase64Decode(const std::string&str);

}  // namespace au

#endif  // APRS_UTILS_UTIL_STRING_H_
