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
#pragma once

#include <stdint.h>

#if defined(CHIBIOS_BUILD)
#include <DC_PAL_ChibiOS/Utils.h>
#elif defined(ARDUPILOT_BUILD)
#include <DC_PAL_ArduPilot/Utils.h>
#elif defined(POSIX_BUILD)
#include <DC_PAL_Posix/Utils.h>
#else
#error "Unknown build target"
#endif

namespace dronecan
{

uint32_t micros32();
uint64_t micros64();

template <typename T, typename ... Args>
T* allocate(Args... args)
{
    auto ret = DC_MALLOC(sizeof(T));
    if (ret == nullptr) {
        return nullptr;
    }
    return new (ret) T(args...);
}

template <typename T>
void deallocate(T *ptr)
{
    ptr->~T();
    DC_FREE(ptr);
}

template <typename T>
constexpr T min(T a, T b)
{
    return a < b ? a : b;
}

template <typename T>
constexpr T max(T a, T b)
{
    return a > b ? a : b;
}

template <typename T>
constexpr T constrain(T x, T a, T b)
{
    return min(max(x, a), b);
}

} // namespace dronecan
