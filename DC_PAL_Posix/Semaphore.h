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

#include <DC_PAL/Semaphore_Abstract.h>
#include <pthread.h>

namespace dronecan
{

class Semaphore : public Semaphore_Abstract
{
public:
    Semaphore();

    ~Semaphore();

    bool take(int32_t timeout_us) override;
    bool give() override;

    pthread_mutex_t *get_mutex()
    {
        return &mtx;
    }
private:
    pthread_mutex_t mtx;
};

} // namespace dronecan