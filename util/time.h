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

#ifndef APRS_UTILS_UTIL_TIME_H_
#define APRS_UTILS_UTIL_TIME_H_

#include <cstdint>

namespace au {

// Gets the current system time in microseconds.
uint64_t GetTimeNowUs();

// Pauses the ucrrent thread for the requested amount of time.
void SleepFor(uint64_t time_us);

// Pauses the current thread until the requested end time.
void SleepUntil(uint64_t end_time_us);

}  // namespace au

#endif  // APRS_UTILS_UTIL_TIME_H_
