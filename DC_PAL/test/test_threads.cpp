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
#include <gtest/gtest.h>

using namespace dronecan;

TEST(Threads, SimpleCreateRun)
{
    int i = 0;
    Thread* main_thread = pal().threads.register_main_thread();

    // create a thread that handles events
    auto func = StaticCallbackFunc([&]() {
        while (true) {
            i++;
            pal().threads.wait(1000); // wait for 1ms
        }
    });

    // start the thread
    auto thd = pal().threads.start(func, "test_thread", 1024, DRONECAN_BASE_PRIO);
    EXPECT_NE(thd, nullptr);

    // sleep for 1 second
    pal().threads.wait(1000000);

    // check number of iterations
    EXPECT_GT(i, 700);
    EXPECT_LT(i, 1100);

    deallocate(thd);
}

TEST(Threads, MultiCreate)
{
    int i = 0;
    Thread* main_thread = pal().threads.register_main_thread();
    Semaphore sem;
    // create a thread that handles events
    auto func = StaticCallbackFunc([&]() {
        while (true) {
            {
                WITH_SEMAPHORE(sem);
                if (i % 2 == 0) {
                    i++;
                }
            }
            pal().threads.wait(1000); // wait for 1ms
        }
    });

    // start the thread
    auto thd = pal().threads.start(func, "test_thread", 1024, DRONECAN_BASE_PRIO);
    EXPECT_NE(thd, nullptr);

    // create another thread that handles events
    auto func2 = StaticCallbackFunc([&]() {
        while (true) {
            {
                WITH_SEMAPHORE(sem);
                if (i % 2 == 1) {
                    i++;
                }
            }
            pal().threads.wait(1000); // wait for 1ms
        }
    });

    // start the thread
    auto thd2 = pal().threads.start(func2, "test_thread2", 1024, DRONECAN_BASE_PRIO);
    EXPECT_NE(thd2, nullptr);

    // sleep for 1 second
    pal().threads.wait(1000000);

    // check number of iterations
    EXPECT_GT(i, 1300);
    EXPECT_LT(i, 2200);

    deallocate(thd);
    deallocate(thd2);
}

TEST(Threads, ThreadEvents)
{
    int i = 0;
    dronecan::event_t evt = -1;
    Thread* main_thread = pal().threads.register_main_thread();
    Semaphore sem;
    // create a thread that handles events
    auto func = StaticCallbackFunc([&]() {
        evt = pal().threads.current()->register_evt();
        dronecan::event_mask_t evt_mask = 1<<evt;
        while (true) {
            i++;
            pal().threads.wait_any(evt_mask, -1); // wait forever
        }
    });
    auto thd = pal().threads.start(func, "test_thread", 1024, DRONECAN_BASE_PRIO);

    auto func2 = StaticCallbackFunc([&]() {
        dronecan::event_mask_t evt_mask = 1<<evt;
        while (true) {
            pal().threads.wait(1000);
            if (evt == -1) {
                continue;
            }
            thd->signal(evt_mask);
        }
    });
    auto thd2 = pal().threads.start(func2, "test_thread2", 1024, DRONECAN_BASE_PRIO);

    pal().threads.wait(1000000);

    // check number of iterations
    EXPECT_GT(i, 700);
    EXPECT_LT(i, 1100);

    deallocate(thd2);
    deallocate(thd);
}
