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

#include "Main.h"
#include <DC_PAL/DC_PAL_Abstract.h>
#include <DC_PAL_Empty/CAN.h>
#include <DC_PAL_Empty/Serial.h>
#include <DC_PAL_Empty/FileSystem.h>
#include "Threads.h"

namespace dronecan::posix
{
class DC_PAL : public dronecan::DC_PAL
{
public:
    DC_PAL(dronecan::empty::FileSystem &fs, dronecan::posix::Threads &threads)
        : dronecan::DC_PAL(fs, threads) {}
};

} // namespace dronecan::posix
