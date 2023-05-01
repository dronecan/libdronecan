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

#include <canard/interface.h>
#include <canard.h>
#include <dronecan/DataIface.h>
#include <dronecan/helpers.h>

#ifndef MAX_ROUTER_INTERFACES
#define MAX_ROUTER_INTERFACES 2
#endif

namespace DroneCAN
{

class Router;
class RouterInterface
{
    friend class Router;
public:
    RouterInterface() {}
    RouterInterface(DataIface *data_iface) : iface(data_iface)
    {
        dronecan_assert(data_iface != nullptr, "RouterInterface: data_iface is nullptr");
        iface->register_cb(route_frame_cb);
    }
    enum InterfaceFlags {
        INTERFACE_RX = 1, // do receive from the Interface
        INTERFACE_TX = 2, // do transmit to the Interface
    };
private:
    void route_frame(CanardCANFrame &frame);
    ObjCallback<RouterInterface, CanardCANFrame&> route_frame_cb{this, &route_frame};
    uint8_t flags;
    uint8_t index;
    DataIface *iface;
};

class Router
{
    friend class RouterInterface;
public:
    Router();
    bool add_interface(DataIface &iface, uint8_t flags);
private:
    RouterInterface interfaces[MAX_ROUTER_INTERFACES];
};

Router& router();

} // namespace DroneCAN
