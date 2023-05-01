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
#if defined(CHIBIOS_BUILD)

#include "DataIface_Raw.h"
#include <ch.hpp>

namespace DroneCAN
{
class ShMemIface : public RawIface
{
public:
    ShMemIface(const ShMemIface&) = delete;
    ShMemIface& operator=(const ShMemIface&) = delete;

    ShMemIface()
    {
        if (_singleton) {
            dronecan_assert(true, "ShMemIface already instantiated");
        }
        _singleton = this;
    }

    bool init() override;

    bool send(const CanardCANFrame &frame) override;
    bool wait_rx(uint32_t wait_us) override;
    void update_rx() override;
    static ShMemIface* get_singleton()
    {
        return _singleton;
    }
    static chibios_rt::EventSource evt_src;
private:
    void signal_rx();
    class HWSemaphore
    {
    public:
        HWSemaphore(uint8_t i);
        ~HWSemaphore();
    private:
        uint16_t unlock_key;
        uint8_t sem_index;
    };

    uint8_t rx_thread_wa[512];
    thread_t *rx_thread;

    static void rx_thread_trampoline(void *arg);
    static ShMemIface *_singleton;
};

} // namespace DroneCAN
#endif // defined(CHIBIOS_BUILD) || defined(ARDUPILOT_BUILD)
