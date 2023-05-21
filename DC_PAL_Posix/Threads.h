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

#include <DC_PAL/Threads.h>
#include <DC_PAL/List.h>
#include <pthread.h>
#include "Semaphore.h"

namespace dronecan::posix
{
class Threads;
class Thread : public dronecan::Thread
{
    friend class Threads;
public:
    Thread(dronecan::Threads::ThreadFunc *thread_func);
    ~Thread();
    void signal(uint32_t events) override;
    dronecan::event_t register_evt() override;
    void unregister_evt(int8_t id) override;
    bool wait_any(uint32_t &events, uint32_t timeout_us) override;
    bool wait_all(uint32_t events, uint32_t timeout_us) override;
    bool set_priority(int priority);
    dronecan::event_mask_t get_registered_events() const override;
private:
    bool start(const char *name, size_t stack_size, int priority);
    dronecan::event_mask_t registered_events;
    dronecan::event_mask_t signalled_events;
    pthread_cond_t cond;
    pthread_t thread;
    dronecan::Threads::ThreadFunc *thread_func;
    pthread_attr_t attr;
    pthread_key_t key;
    Semaphore sem;
    bool attr_init;
    bool cond_init;
};

class Threads : public dronecan::Threads
{
    friend class Thread;
public:
    Threads();
    ~Threads();
    Thread* start(dronecan::Threads::ThreadFunc &thread_func, const char *name, size_t stack_size, int priority) override;
    Thread* current() override;
    bool wait_any(dronecan::event_mask_t &evt_mask, uint32_t timeout_us) override;
    bool wait_all(dronecan::event_mask_t evt_mask, uint32_t timeout_us) override;
    void stop(dronecan::Thread* thread) override;
    Thread* register_main_thread() override;
private:
    static pthread_key_t key;
    Thread *main_thread;
};

} // namespace dronecan
