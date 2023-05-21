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
#include "Worker.h"
#include "DC_PAL.h"

using namespace dronecan;

Worker::~Worker()
{
    if (thread) {
        // unregister all workers
        WorkerObject* curr_worker = worker_obj_head;
        while (curr_worker) {
            WorkerObject* next_worker = curr_worker->next;
            unregister_worker(curr_worker);
            curr_worker = next_worker;
        }
        deallocate(thread);
    }
}

bool Worker::start(const char *name, size_t stack_size, int priority)
{
    WITH_SEMAPHORE(sem);
    // start a worker thread
    thread = pal().threads.start(thread_func, name, stack_size, priority);
    if (thread == nullptr) {
        dronecan_assert(false, "Failed to initialise worker %s", name)
        return false;
    }
    return true;
}

uint64_t Worker::get_next_trigger_us()
{
    WITH_SEMAPHORE(sem);
    uint64_t next_trigger_us = 0;
    WorkerObject* curr_worker = worker_obj_head;
    while (curr_worker) {
        if (curr_worker->worker_type != WorkerObject::WORKER_TYPE_PERIOD) {
            curr_worker = curr_worker->next;
            continue;
        }
        PeriodicWorkerObject* periodic_curr_worker = (PeriodicWorkerObject*)curr_worker;

        if ((next_trigger_us == 0) ||
            (next_trigger_us > periodic_curr_worker->get_next_trigger_us())) {
            next_trigger_us = periodic_curr_worker->get_next_trigger_us();
        }
        curr_worker = curr_worker->next;
    }
    return next_trigger_us;
}

void Worker::run()
{
    running = true;
    event_mask_t evt_msk = 0;
    while (true) {
        {
            WITH_SEMAPHORE(sem);
            WorkerObject* curr_worker = worker_obj_head;
            while (curr_worker) {
                (*curr_worker)(micros64());
                (*curr_worker)(evt_msk);
                curr_worker = curr_worker->next;
            }
        }
        uint64_t next_trigger_us = get_next_trigger_us();
        uint64_t curr_time_us = micros64();
        evt_msk = thread->get_registered_events();
        if (next_trigger_us < curr_time_us) {
            pal().threads.wait_any(evt_msk, -1);
        } else {
            pal().threads.wait_any(evt_msk, next_trigger_us - curr_time_us);
        }
    }
}

bool Worker::register_worker(WorkerObject *worker_obj)
{
    WITH_SEMAPHORE(sem);
    worker_obj->set_worker(this);
    if (worker_obj_head == nullptr) {
        worker_obj_head = worker_obj;
        return true;
    }

    WorkerObject* curr_worker_obj = worker_obj_head;
    while (curr_worker_obj) {
        if (curr_worker_obj->next == nullptr) {
            curr_worker_obj->next = worker_obj;
            return true;
        }
        curr_worker_obj = curr_worker_obj->next;
    }
    dronecan_assert(false, "Should be unreachable");
    return false;
}

void Worker::unregister_worker(WorkerObject *worker_obj)
{
    WITH_SEMAPHORE(sem);
    if (worker_obj_head == nullptr) {
        return;
    }
    if (worker_obj_head == worker_obj) {
        worker_obj_head->set_worker(nullptr);
        worker_obj_head = worker_obj_head->next;
        return;
    }

    WorkerObject* curr_worker_obj = worker_obj_head;
    while (curr_worker_obj) {
        if (curr_worker_obj->next == worker_obj) {
            curr_worker_obj->next->set_worker(nullptr);
            curr_worker_obj->next = curr_worker_obj->next->next;
            return;
        }
    }
    dronecan_assert(false, "Should be unreachable");
}
