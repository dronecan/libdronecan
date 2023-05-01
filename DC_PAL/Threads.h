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

#include <stdint.h>
#include <unistd.h>
#include "Callbacks.h"
#include "Utils.h"
#include "List.h"

namespace dronecan
{
typedef uint32_t event_mask_t;
typedef int8_t event_t;

class Thread : public List<Thread>
{
public:
    virtual ~Thread() = default;
    virtual void signal(dronecan::event_mask_t event_mask) = 0;
    virtual dronecan::event_t register_evt() = 0;
    virtual void unregister_evt(int8_t id) = 0;
    virtual bool wait_any(dronecan::event_mask_t events, uint32_t timeout_us) = 0;
    virtual bool wait_all(dronecan::event_mask_t events, uint32_t timeout_us) = 0;
};

class Threads
{
public:
    using ThreadFunc = Callback<>;

    Threads()
    {
        if (instance != nullptr) {
            dronecan_assert(false, "Threads already initialized");
        }
        instance = this;
    }

    virtual Thread* start(ThreadFunc &thread_func, const char *name, size_t stack_size, int priority) = 0;
    virtual Thread* current() = 0;
    virtual bool wait(uint32_t evt_mask, uint32_t timeout_us) = 0;
    virtual void stop(Thread* thread) = 0;
    virtual Thread* register_main_thread()
    {
        return nullptr;
    }
private:
    Threads *instance;
};

} // namespace dronecan
