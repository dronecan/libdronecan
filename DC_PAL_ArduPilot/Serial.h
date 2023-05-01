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

#include <DC_PAL/Serial.h>
#include <DC_PAL/List.h>
#include <DC_PAL/Threads.h>

namespace dronecan::ardupilot
{

class Serial : public dronecan::Serial
{
public:
    Serial(uint8_t _instance) : instance(_instance) {}
    bool init() override;

    bool send(uint8_t* payload, size_t length) override;

    void update_rx() override;

    // returns mask to be used with threads.wait(mask, timeout_us)
    uint32_t register_wait_rx(size_t num_bytes, uint32_t timeout_us) override;
    void unregister_wait_rx(uint32_t id) override;
private:
    uint8_t instance;
    AP_HAL::UARTDriver* serial;
    static bool serial_manager_initialised;
    List<Thread> rx_threads;
};

} // namespace dronecan