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

#ifndef APRS_FILE_COPY_UTIL_FILE_H_
#define APRS_FILE_COPY_UTIL_FILE_H_

#include <string>

namespace afc {

// Read the contents of the supplied file into a string.
// Returns true if successful, false otherwise.
bool ReadFileToString(const std::string& filename, std::string* contents);

// Write the supplied string to a file.
// Returns true if successful, false otherwise.
bool WriteStringToFile(const std::string& filename,
    const std::string& contents, int mode = 0600, bool fail_if_exists = false);

// Writes the supplied buffer to a file.
// Returns true if successful, false otherwise.
bool WriteBufferToFile(const std::string& filename,
    const uint8_t* buffer, size_t size, int mode = 0600,
    bool fail_if_exists = false);

}  // namespace afc

#endif  // APRS_FILE_COPY_UTIL_FILE_H_
