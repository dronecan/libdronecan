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

#include "Callbacks.h"
#include <canard.h>

namespace DroneCAN
{

// DataIface to CanardIface
class DataIface
{
protected:
    CallbackList<CanardCANFrame&> rx_cbs;
public:
    bool register_cb(Callback<CanardCANFrame&> &cb)
    {
        return rx_cbs.register_cb(cb);
    }
    void unregister_cb(Callback<CanardCANFrame&> &cb)
    {
        rx_cbs.unregister_cb(cb);
    }
    virtual bool init() = 0;
    virtual bool send(const CanardCANFrame &frame)
    {
        if (flags & INTERFACE_WRITE) {
            return true;
        }
        return this->send(frame);
    }
    virtual void update_rx() = 0;
    virtual bool wait_rx(uint32_t wait_us) = 0;
};

} // namespace DroneCAN
