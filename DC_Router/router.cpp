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

#include "router.h"

using namespace DroneCAN;

Router *Router::singleton = nullptr;

Router::Router()
{
    if (singleton != nullptr) {
        dronecan_assert(true, "Router already instantiated");
    }
    singleton = this;
}

bool Router::add_interface(DataIface &iface, uint8_t flags)
{
    for (auto &i : interfaces) {
        if (i.iface == &iface) {
            // already added
            return true;
        } else if (i.iface == nullptr) {
            // found a free slot
            i.iface = &iface;
            i.flags = flags;
            return true;
        }
    }
    return false;
}

void RouterInterface::route_frame(CanardCANFrame &frame)
{
    for (auto &i : router().interfaces) {
        // route to all interfaces except the one it came from
        if (i.iface != nullptr && i.flags & INTERFACE_TX && i.iface != iface) {
            i.iface->send(frame);
        }
    }
}
