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

namespace dronecan
{

class Semaphore_Abstract
{
public:
    virtual bool take(int32_t timeout_us) = 0;
    virtual bool give() = 0;
};

class WithSemaphore
{
public:
    WithSemaphore(Semaphore_Abstract& sem) : _sem(sem)
    {
        _sem.take(-1);
    }
    ~WithSemaphore()
    {
        _sem.give();
    }
private:
    Semaphore_Abstract& _sem;
};

#define WITH_SEMAPHORE(sem) JOIN( sem, __COUNTER__ )

#define JOIN( sem, counter ) _DO_JOIN( sem, counter )
#define _DO_JOIN( sem, counter ) WithSemaphore _getsem ## counter(sem)

} // namespace dronecan