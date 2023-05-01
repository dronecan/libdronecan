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

#include <DC_PAL/Callbacks.h>
#include <cstdio>
#include <gtest/gtest.h>

using namespace dronecan;

class TestCallback
{
    Callback<int> cb;
public:

    template <typename func, typename Obj>
    using ObjFunc = ObjCallback<func, Obj, int>;

    template <typename func>
    using StaticFunc = StaticCallback<func, int>;

    void register_cb(Callback<int> &_cb)
    {
        cb.register_cb(_cb);
    }

    void unregister_cb(Callback<int> &_cb)
    {
        cb.unregister_cb(_cb);
    }

    void operator()(int i)
    {
        cb(i);
    }
};

static int expect_val = 0;
class RegisterCallback
{
public:
    static int count;
    void test_func(int i)
    {
        EXPECT_EQ(i, expect_val);
        count++;
    }
    TestCallback::ObjFunc<void (RegisterCallback::*)(int), RegisterCallback> cb{this, &RegisterCallback::test_func};
};

int RegisterCallback::count = 0;

// add google test here
TEST(Callbacks, RegisterCallback)
{
    RegisterCallback rc1, rc2, rc3;
    TestCallback tc;
    tc.register_cb(rc1.cb);
    tc.register_cb(rc2.cb);
    tc.register_cb(rc3.cb);
    expect_val = 1;
    tc(1);
    EXPECT_EQ(RegisterCallback::count, 3);
    tc.unregister_cb(rc2.cb);
    expect_val = 2;
    tc(2);
    EXPECT_EQ(RegisterCallback::count, 5);
}
