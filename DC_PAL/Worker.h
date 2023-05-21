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
#include "Callbacks.h"
#include "Threads.h"
#include <iostream>

namespace dronecan
{
using WorkerFunc = Callback<>;

class WorkerObject;
class Worker;
class Worker
{
public:
    Worker() : thread_func(this, &Worker::run)
    {}

    ~Worker();

    bool start(const char *name, size_t stack_size, int priority);

    bool register_worker(WorkerObject *worker_obj);

    void unregister_worker(WorkerObject *worker_obj);

    void run();
    bool is_running() const
    {
        return running;
    }

    event_t register_evt()
    {
        return thread->register_evt();
    }

    void signal(dronecan::event_mask_t event_mask)
    {
        thread->signal(event_mask);
    }

private:
    uint64_t get_next_trigger_us();
    event_mask_t get_combined_evt_mask();
    bool running;
    WorkerObject *worker_obj_head;
    ObjCallbackFunc<Worker, void (Worker::*)()> thread_func;
    Thread* thread;
    Semaphore sem;
};

class WorkerObject
{
    friend class Worker;
public:
    enum WorkerType {
        WORKER_TYPE_PERIOD,
        WORKER_TYPE_EVT
    };

    WorkerObject(WorkerFunc *_worker_func, WorkerType _worker_type) :
        worker_func(_worker_func),
        next(nullptr),
        worker_type(_worker_type)
    {}

    ~WorkerObject()
    {
        if (worker) {
            worker->unregister_worker(this);
        }
    }
    virtual void operator()(event_mask_t _evt_mask) = 0;
    virtual void operator()(uint64_t curr_time) = 0;
protected:
    virtual void set_worker(Worker* _worker)
    {
        worker = _worker;
    }
    WorkerFunc *worker_func;
    WorkerObject* next;
    Worker *worker;
    WorkerType worker_type;
};

class PeriodicWorkerObject : public WorkerObject
{
public:
    PeriodicWorkerObject(WorkerFunc *_worker_func, uint32_t _period_us) :
        WorkerObject(_worker_func, WorkerObject::WORKER_TYPE_PERIOD), period_us(_period_us)
    {}

    uint64_t get_next_trigger_us() const
    {
        return next_trigger_time_us;
    }
    void operator()(event_mask_t _evt_mask) override {}

    void operator()(uint64_t curr_time) override
    {
        if (curr_time >= next_trigger_time_us) {
            (*worker_func)();
            next_trigger_time_us = period_us + micros64();
        }
    }
private:
    void set_worker(Worker* _worker) override
    {
        WorkerObject::set_worker(_worker);
        next_trigger_time_us = period_us + micros64();
    }
    uint32_t period_us;
    uint64_t next_trigger_time_us;
};

class EventWorkerObject : public WorkerObject
{
public:
    EventWorkerObject(WorkerFunc *_worker_func, event_mask_t _evt_mask, bool any) :
        WorkerObject(_worker_func, WorkerObject::WORKER_TYPE_EVT),
        evt_mask(_evt_mask),
        trigger_any(any)
    {}

    event_mask_t get_event_mask() const
    {
        return evt_mask;
    }

    void operator()(event_mask_t _evt_mask) override
    {
        if ((evt_mask & _evt_mask) == evt_mask) {
            (*worker_func)();
        } else if ((evt_mask & _evt_mask) && trigger_any) {
            (*worker_func)();
        }
    }

    void operator()(uint64_t curr_time) override {}
private:
    event_mask_t evt_mask;
    bool trigger_any;
};

} // namespace DroneCAN
