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
#include <pthread.h>

using namespace dronecan;

TEST(Worker, TestWorker)
{
    Worker *worker = allocate<Worker>();
    worker->start("test_worker", 1024, DRONECAN_BASE_PRIO + 1);
    Thread* main_thread = pal().threads.register_main_thread();
    uint64_t signal_time;
    // set this thread to high priority
    main_thread->set_priority(DRONECAN_BASE_PRIO);

    uint64_t avg_period_us = 0;
    uint64_t last_run_us = 0;
    int num_runs = 0;
    auto periodic_func1 = StaticCallbackFunc([&]() {
        uint64_t now = dronecan::micros64();
        if (last_run_us != 0) {
            uint32_t period_us = now - last_run_us;
            avg_period_us = ((avg_period_us * num_runs) + period_us) / (num_runs + 1);
        }
        last_run_us = now;
        num_runs++;
    });

    class PeriodicWorker
    {
    public:
        uint64_t avg_period_us = 0;
        uint64_t last_run_us = 0;
        int num_runs = 0;
        void periodic_cb()
        {
            uint64_t now = dronecan::micros64();
            if (last_run_us != 0) {
                uint32_t period_us = now - last_run_us;
                avg_period_us = ((avg_period_us * num_runs) + period_us) / (num_runs + 1);
            }
            last_run_us = now;
            num_runs++;
        }
    } periodic_worker;

    auto periodic_func2 = ObjCallbackFunc(&periodic_worker, &PeriodicWorker::periodic_cb);

    int evt_cb_runs1 = 0;
    auto evt_func1 = StaticCallbackFunc([&]() {
        EXPECT_NEAR(dronecan::micros64() - signal_time, 0, 100);
        evt_cb_runs1++;
    });
    int evt_cb_runs2 = 0;
    auto evt_func2 = StaticCallbackFunc([&]() {
        EXPECT_NEAR(dronecan::micros64() - signal_time, 0, 100);
        evt_cb_runs2++;
    });

    PeriodicWorkerObject pw1{&periodic_func1, 100000};
    PeriodicWorkerObject pw2{&periodic_func2, 100000};

    event_t evt1 = worker->register_evt();
    event_t evt2 = worker->register_evt();
    EventWorkerObject ew1{&evt_func1, dronecan::events(evt1), true};
    EventWorkerObject ew2{&evt_func2, dronecan::events(evt2), true};

    // register workers
    worker->register_worker(&pw1);
    worker->register_worker(&pw2);
    worker->register_worker(&ew1);
    worker->register_worker(&ew2);
    uint64_t start_time = dronecan::micros64();
    // generate 5 events
    for (int i = 0; i < 5; i++) {
        signal_time = dronecan::micros64();
        worker->signal(dronecan::events(evt1, evt2));
        pal().threads.wait(100000);
    }

    uint64_t total_run_time = dronecan::micros64() - start_time;
    // check that the periodic callbacks were called
    EXPECT_NEAR(avg_period_us, 100000, 30000);
    EXPECT_NEAR(periodic_worker.avg_period_us, 100000, 30000);

    // check that the events were received
    EXPECT_EQ(evt_cb_runs1, 5);
    EXPECT_EQ(evt_cb_runs2, 5);

    deallocate(worker);
}
