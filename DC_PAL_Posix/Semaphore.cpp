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
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&mtx, &attr);
}

Semaphore::~Semaphore()
{
    pthread_mutex_destroy(&mtx);
}

int64_t time_diff_us(struct timespec &ts1, struct timespec &ts2)
{
    return 1.0e6*((ts1.tv_sec + (ts1.tv_nsec*1.0e-9)) -
                  (ts2.tv_sec + (ts2.tv_nsec*1.0e-9)));
}

// compat pthread_mutex_timedlock
int __attribute__((weak)) pthread_mutex_timedlock(pthread_mutex_t *mtx, struct timespec *ts)
{
    struct timespec curr_ts;
    clock_gettime(CLOCK_REALTIME, &curr_ts);
    while (time_diff_us(curr_ts, *ts) > 0) {
        if (pthread_mutex_trylock(mtx) == 0) {
            return 0;
        }
        struct timespec sleep_ts;
        sleep_ts.tv_sec = 0;
        sleep_ts.tv_nsec = 200000; // 200us sleep
        nanosleep(&sleep_ts, nullptr);
        clock_gettime(CLOCK_REALTIME, &curr_ts);
    }
    return pthread_mutex_trylock(mtx);
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
