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
#include <Callbacks.h>

namespace dronecan
{

class WorkerObject
{
public:
    WorkerObject(Callback<WorkerObject&> &cb) : cb(cb) {}
    virtual bool init_periodic(uint32_t period_us) = 0;
    virtual int8_t init_event() = 0;
    virtual bool operator()()
    {
        return cb(*this);
    }
private:
    Callback<WorkerObject&> cb;
};

class Worker
{
public:
    Worker(const char *name, size_t stack_size, int priority);

    bool register_cb(Callback<WorkerObject&> &cb)
    {
        return worker_cbs.register_cb(cb);
    }
    void unregister_cb(Callback<WorkerObject&> &cb)
    {
        worker_cbs.unregister_cb(cb);
    }
    bool start();
    void update();
    bool add(WorkerObject &obj);
    bool remove(WorkerObject &obj);
    bool is_running() const;

private:
    bool running;
    List<WorkerObject> worker_cbs;
    Thread thread;
};

} // namespace DroneCAN
