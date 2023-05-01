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

#include "ch.h"
#include "Semaphore.h"

namespace dronecan::chibios
{

class Events
{
public:
    int8_t register(Thread* thread) override;
    bool unregister(uint8_t id) override;
    bool wait_any(uint32_t timeout_us) override;
    bool wait_all(uint32_t timeout_us) override;

    void signal(uint8_t id);
private:
    thread_t *thread;
    eventflags_t event_flags;
    Semaphore sem;
};

} // namespace dronecan