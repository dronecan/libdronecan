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

#include "List.h"
#include "FileSystem.h"
#include "Serial.h"
#include "CAN.h"
#include "Threads.h"
#include "Utils.h"

namespace dronecan
{

class DC_PAL
{
public:
    DC_PAL(FileSystem &fs, Threads &threads)
        : fs(fs), threads(threads), can(CAN::get_head()), serial(Serial::get_head())
    {
        if (instance != nullptr) {
            dronecan_assert(false, "DC_PAL already instantiated");
        }
        instance = this;
    }

    CAN* const &can;
    Serial* const &serial;
    FileSystem& fs;
    Threads& threads;

    static DC_PAL* get_singleton()
    {
        return instance;
    }
private:
    static DC_PAL *instance;
};

DC_PAL& pal();

} // namespace dronecan