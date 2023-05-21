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

namespace dronecan
{

/// @brief Base class for callbacks.
template <typename ...Args>
class Callback
{
    Callback<Args...> *next;
public:
    Callback() : next(nullptr) {}

    bool register_cb(Callback<Args...> &_cb)
    {
        if (next == &_cb) {
            // already registered
            return true;
        }
        auto next_cb = this;
        while (next_cb->next != nullptr) {
            if (next_cb->next == &_cb) {
                // already registered
                return true;
            }
            next_cb = next_cb->next;
        }
        next_cb->next = &_cb;
        next_cb->next->next = nullptr;
        return true;
    }

    void unregister_cb(Callback<Args...> &_cb)
    {
        if (next == &_cb) {
            next = next->next;
            return;
        }
        auto next_cb = this;
        while (next_cb->next != nullptr) {
            if (next_cb->next == &_cb) {
                next_cb->next = next_cb->next->next;
                return;
            }
            next_cb = next_cb->next;
        }
    }

    // @brief call all registered callbacks
    virtual void call_all(Args...args)
    {
        auto next_cb = next;
        while (next_cb != nullptr) {
            (*next_cb)(args...);
            next_cb = next_cb->next;
        }
    }

    Callback<Args...> *get_next()
    {
        return next;
    }

    virtual void operator()(Args...args) = 0;
};

template <typename func, typename ...Args>
class StaticCallbackFunc : public Callback<Args...>
{
    func cb;
public:
    /// @brief constructor
    /// @param _cb callback function
    StaticCallbackFunc(func _cb) : cb(_cb) {}

    void operator()(Args...args) override
    {
        cb(args...);
    }

    operator func()
    {
        return cb;
    }
};

template <typename type, typename func, typename ...Args>
class ObjCallbackFunc : public Callback<Args...>
{
    type *obj;
    func cb;
public:
    /// @brief constructor
    /// @param _cb callback function
    ObjCallbackFunc(type *_obj, func _cb) : obj(_obj), cb(_cb) {}

    void operator()(Args...args) override
    {
        if (obj != nullptr) {
            (obj->*cb)(args...);
        }
    }
};

} // namespace DroneCAN