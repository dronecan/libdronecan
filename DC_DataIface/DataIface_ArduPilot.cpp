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

#include "DataIface_ArduPilot.h"
#if defined(ARDUPILOT_BUILD)

#include <AP_HAL/AP_HAL.h>
#include "canard.h"

extern const AP_HAL::HAL& hal;
using namespace DroneCAN;

ArduPilotIface *ArduPilotIface::singleton = nullptr;

bool ArduPilotIface::init()
{
    if (hal.can[instance] == nullptr) {
        return false;
    }
    return hal.can[instance]->init(bitrate, fdbitrate*1000000, mode);
}

bool ArduPilotIface::send(const CanardCANFrame &frame)
{
    if (hal.can[instance] == nullptr) {
        return false;
    }
    AP_HAL::CANFrame txmsg;
    txmsg.dlc = dataLengthToDlc(txf->data_len);
    memcpy(txmsg.data, txf->data, txf->data_len);
    txmsg.id = (txf->id | AP_HAL::CANFrame::FlagEFF);
    txmsg.canfd = txf->canfd;
    bool write = true;
    bool read = false;
    hal.can[instance]->select(read, write, &txmsg, 0);
    if ((AP_HAL::native_micros64() < txf->deadline_usec) && (txf->iface_mask & (1U<<instance)) && write) {
        // try sending to interfaces, clearing the mask if we succeed
        if (hal.can[instance]->send(txmsg, txf->deadline_usec, 0) > 0) {
            txf->iface_mask &= ~(1U<instance);
            return true;
        }
    } else {
        // we missed the deadline, clear the mask
        txf->iface_mask &= ~(1U<<instance);
        return false
    }
    return false;
}

void ArduPilotIface::update_rx()
{
    if (hal.can[instance] == nullptr) {
        return;
    }
    AP_HAL::CANFrame rxmsg;
    bool write = false;
    bool read = true;
    hal.can[instance]->select(read, write, &rxmsg, 0);
    if (read) {
        CanardCANFrame frame;
        frame.id = rxmsg.id;
        frame.data_len = dlcToDataLength(rxmsg.dlc);
        memcpy(frame.data, rxmsg.data, rxmsg.dlc);
        frame.canfd = rxmsg.canfd;
        frame.timestamp_usec = AP_HAL::native_micros64();
        rx_cbs(frame);
    }
}

bool ArduPilotIface::wait_rx(uint32_t wait_us)
{
    if (hal.can[instance] == nullptr) {
        return;
    }
    bool write = false;
    bool read = true;
    return hal.can[instance]->select(read, write, nullptr,
                                     AP_HAL::native_micros64() + wait_us);
}

#endif // ARDUPILOT_BUILD
