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
#include "DataIface_Abstract.h"
#include "helpers.h"
#include "AP_HAL/AP_HAL.h"

namespace DroneCAN
{
class ArduPilotIface : public DataIface
{
public:
    ArduPilotIface(uint8_t instance, uint16_t _bitrate, uint16_t _fdbitrate, AP_HAL::CANIface::OperatingMode _mode) :
        bitrate(_bitrate),
        fdbitrate(_fdbitrate),
        mode(_mode),
        DataIface()
    {
        if (singleton != nullptr) {
            dronecan_assert(true, "ArduPilotIface already instantiated");
        }
        singleton = this;
    }

    bool init() override;
    bool send(const CanardCANFrame &frame) override;
    void update_rx() override;
    bool wait_rx(uint32_t wait_us) override;

    static ArduPilotIface* get_singleton()
    {
        return singleton;
    }
private:
    static ArduPilotIface *singleton;
    AP_HAL::CANIface *can_drv;
    uint8_t instance;
    uint16_t bitrate;
    uint16_t fdbitrate;
    AP_HAL::CANIface::OperatingMode mode;
};
} // namespace DroneCAN
