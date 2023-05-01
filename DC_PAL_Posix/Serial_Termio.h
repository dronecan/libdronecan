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

#include <DC_PAL/Serial.h>
#include <termio.h>
#include "Threads.h"

namespace dronecan::posix
{
class Serial_Termio : public Serial
{
public:
    Serial_Termio(const char *port, uint32_t baudrate, uint32_t flags) :
        port(port),
        baudrate(baudrate),
        flags(flags)
    {}

    bool init() override;
    bool send(uint8_t* payload, size_t length) override;

    // wait for num_bytes to be received, or timeout_us to expire
    void update_rx() override;
    uint32_t register_wait_rx(size_t num_bytes, uint32_t timeout_us) override;
    void unregister_wait_rx(uint32_t id) override;
private:
    int fd;
    const char* port;
    uint32_t baudrate;
    uint32_t flags;
    Thread* rx_thread;
    void rx_thread_func();
    OBJ_CALLBACK(rx_thread_func_cb, rx_thread_func, this);
};

} // namespace dronecan::posix

