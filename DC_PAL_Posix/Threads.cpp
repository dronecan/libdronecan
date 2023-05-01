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

#include <DC_PAL/DC_PAL.h>
#include "Threads.h"
#include "Semaphore.h"
#include "Utils.h"
#include <errno.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace dronecan::posix;

Thread::Thread(dronecan::Threads::ThreadFunc *_thread_func) : thread_func(_thread_func)
{}

Thread::~Thread()
{
    if (thread_func != nullptr) {
        pthread_cancel(thread);
        pthread_join(thread, nullptr);
    }
    if (attr_init) {
        pthread_attr_destroy(&attr);
    }
    if (cond_init) {
        pthread_cond_destroy(&cond);
    }
}

bool Thread::start(const char *name, size_t stack_size, int priority)
{
    if (thread_func == nullptr) {
        // just create condition variable to wait on
        // this thread is not managed by us
        int ret = pthread_cond_init(&cond, nullptr);
        if (ret != 0) {
            dronecan_assert(false, "pthread_cond_init %s", strerror(ret));
            return false;
        }
        cond_init = true;
        return true;
    }

    int ret = pthread_attr_init(&attr);
    if (ret != 0) {
        dronecan_assert(false, "pthread_attr_init %s", strerror(ret));
        return false;
    }
    attr_init = true;

    ret = pthread_cond_init(&cond, nullptr);
    if (ret != 0) {
        dronecan_assert(false, "pthread_cond_init %s", strerror(ret));
        return false;
    }
    cond_init = true;

    // set the stack size
    ret = pthread_attr_setstacksize(&attr, PTHREAD_STACK_MIN + stack_size);
    if (ret != 0) {
        dronecan_assert(false, "pthread_attr_setstacksize %s", strerror(ret));
        return false;
    }

    // if we have permissions, set the priorities
    if (geteuid() == 0) {
        // set the priority
        struct sched_param param;
        ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
        if (ret != 0) {
            dronecan_assert(false, "pthread_attr_setinheritsched %s", strerror(ret));
            return false;
        }
        ret = pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
        if (ret != 0) {
            dronecan_assert(false, "pthread_attr_setschedpolicy %s", strerror(ret));
            return false;
        }
        priority = DRONECAN_PRIO(priority);
        param.sched_priority = constrain(priority, sched_get_priority_min(SCHED_FIFO), sched_get_priority_max(SCHED_FIFO));
        ret = pthread_attr_setschedparam(&attr, &param);
        if (ret != 0) {
            dronecan_assert(false, "pthread_attr_setschedparam %s", strerror(ret));
            return false;
        }
    }

    ret = pthread_create(&thread, &attr, [](void* arg) -> void* {
        Thread* thd = (Thread*)arg;
        pthread_setspecific(Threads::key, thd);
        if (thd->thread_func)
        {
            (*thd->thread_func)();
        }
        return nullptr;
    }, this);
    if (ret != 0) {
        dronecan_assert(false, "pthread_create %s", strerror(ret));
        return false;
    }

    return true;
}

void Thread::signal(uint32_t events)
{
    WITH_SEMAPHORE(sem);
    // check if the evt is registered
    if (events & registered_events) {
        signalled_events |= events;
        pthread_cond_signal(&cond);
    }
}

dronecan::event_t Thread::register_evt()
{
    WITH_SEMAPHORE(sem);
    if (registered_events == 0xFFFFFFFF) {
        return -1;
    }

    dronecan::event_t id = 0;
    while ((registered_events & (1 << id)) && id < 32) {
        id++;
    }
    if (id == 32) {
        return -1;
    }
    return id;
}

void Thread::unregister_evt(int8_t id)
{
    WITH_SEMAPHORE(sem);
    registered_events &= ~(1 << id);
}

bool Thread::wait_any(dronecan::event_mask_t events, uint32_t timeout_us)
{
    WITH_SEMAPHORE(sem);
    if (signalled_events & events) {
        signalled_events &= ~events;
        return true;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += timeout_us * 1000;
    ts.tv_sec += ts.tv_nsec / 1000000000;
    ts.tv_nsec %= 1000000000;

    while (true) {
        int ret = pthread_cond_timedwait(&cond, sem.get_mutex(), &ts);
        if (ret == ETIMEDOUT) {
            signalled_events = 0;
            return false;
        }
        if (signalled_events & events) {
            signalled_events = 0;
            return true;
        }
    }
}

bool Thread::wait_all(dronecan::event_mask_t events, uint32_t timeout_us)
{
    WITH_SEMAPHORE(sem);
    if ((signalled_events & events) == events) {
        signalled_events &= ~events;
        return true;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += timeout_us * 1000;
    ts.tv_sec += ts.tv_nsec / 1000000000;
    ts.tv_nsec %= 1000000000;

    while (true) {
        int ret = pthread_cond_timedwait(&cond, sem.get_mutex(), &ts);
        if (ret == ETIMEDOUT) {
            signalled_events = 0;
            return false;
        }
        if ((signalled_events & events) == events) {
            signalled_events = 0;
            return true;
        }
    }
}

pthread_key_t Threads::key;

Threads::Threads()
{
    pthread_key_create(&key, nullptr);
}

Threads::~Threads()
{
    if (Thread::get_head() != nullptr) {
        // destroy all threads
        for (auto thd : *Thread::get_head()) {
            deallocate(thd);
        }
    }
    pthread_key_delete(key);
}

Thread* Threads::start(dronecan::Threads::ThreadFunc &thread_func, const char *name, size_t stack_size, int priority)
{
    // set up the thread
    Thread* thread = allocate<Thread>(&thread_func);
    if (thread == nullptr) {
        return nullptr;
    }
    if (thread->start(name, stack_size, priority) == false) {
        deallocate(thread);
        return nullptr;
    }
    return thread;
}

void Threads::stop(dronecan::Thread* _thread)
{
    auto thread = (dronecan::posix::Thread*)_thread;
    deallocate(thread);
}

Thread* Threads::current()
{
    return (Thread*)pthread_getspecific(key);
}

bool Threads::wait(dronecan::event_mask_t evt_mask, uint32_t timeout_us)
{
    Thread* thd = current();
    if (thd == nullptr) {
        return false;
    }
    return thd->wait_any(evt_mask, timeout_us);
}

Thread* Threads::register_main_thread()
{
    main_thread = allocate<Thread>(nullptr);
    if (main_thread == nullptr) {
        return nullptr;
    }
    main_thread->start(nullptr, 0, 0);
    pthread_setspecific(key, main_thread);
    return main_thread;
}
