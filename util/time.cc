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

#include "util/time.h"

#include <chrono>

#include <unistd.h>

namespace au {

uint64_t GetTimeNowUs() {
  return std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count();
}

void SleepFor(uint64_t time_us) {
  SleepUntil(GetTimeNowUs() + time_us);
}

void SleepUntil(uint64_t end_time_us) {
  uint64_t time_now_us = GetTimeNowUs();
  if (end_time_us > time_now_us) {
    usleep(end_time_us - time_now_us);
  }
}

}  // namespace au
