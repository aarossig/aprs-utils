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

#ifndef APRS_UTILS_UTIL_LOG_H_
#define APRS_UTILS_UTIL_LOG_H_

#include <cstdio>

// Logging macros to put logs on stderr.

// TODO(aarossig): None of this is thread safe.

// Informational logs.
#define LOGI(format, ...) \
  fprintf(stderr, LOG_TAG ": " format "\n", ##__VA_ARGS__)

// Error logs.
#define LOGE(format, ...) \
  fprintf(stderr, LOG_TAG ": " format "\n", ##__VA_ARGS__)

// Fatal logs.
#define LOGFATAL(format, ...) do { \
    fprintf(stderr, LOG_TAG ": " format "\n", ##__VA_ARGS__); \
    exit(-1); \
  } while (0)

#endif  // APRS_UTILS_UTIL_LOG_H_
