/*
Copyright 2023 Siddharth Bharat Purohit, Cubepilot Pty Ltd.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#include <DC_PAL/Utils.h>
#include "hal.h"

using namespace dronecan;

uint32_t micros32()
{
    systime_t time = chVTGetSystemTimeX();
    time *= 1000000/CH_CFG_ST_FREQUENCY;
    return time;
}

uint64_t micros64()
{
    static uint32_t last_time = 0;
    static uint32_t overflow_count = 0;
    uint32_t time = micros32();
    if (time < last_time) {
        overflow_count++;
    }
    last_time = time;
    return ((uint64_t)overflow_count << 32) | (uint64_t)time;
}
