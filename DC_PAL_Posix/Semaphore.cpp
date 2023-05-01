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

#include "Semaphore.h"
#include <time.h>

using namespace dronecan;

Semaphore::Semaphore()
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
    pthread_mutex_init(&mtx, &attr);
}

Semaphore::~Semaphore()
{
    pthread_mutex_destroy(&mtx);
}

bool Semaphore::take(int32_t timeout_us)
{
    if (timeout_us == -1) {
        // Block forever
        return pthread_mutex_lock(&mtx) == 0;
    } else if (timeout_us == 0) {
        // Non-blocking
        return pthread_mutex_trylock(&mtx) == 0;
    } else {
        // Block for a certain amount of time
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_sec += timeout_us / 1000000;
        ts.tv_nsec += (timeout_us % 1000000) * 1000;
        return pthread_mutex_timedlock(&mtx, &ts) == 0;
    }
}

bool Semaphore::give()
{
    return pthread_mutex_unlock(&mtx) == 0;
}
