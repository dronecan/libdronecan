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

#include "Semaphore.h"

namespace dronecan
{

template <typename T>
class List
{
    T *next;
    static T* head;
    static Semaphore sem;
public:
    List() : next(nullptr)
    {
        add(*(T*)this);
    }

    ~List()
    {
        remove(*(T*)this);
    }

    void add(T &obj)
    {
        WITH_SEMAPHORE(sem);
        if (head == nullptr) {
            head = &obj;
        } else {
            T *tmp = head;
            while (tmp->next != nullptr) {
                tmp = tmp->next;
            }
            tmp->next = &obj;
        }
    }

    void remove(T &obj)
    {
        WITH_SEMAPHORE(sem);
        if (head == nullptr) {
            // nothing to remove
            return;
        }
        if (head == &obj) {
            head = obj.next;
        } else {
            T *tmp = head;
            while (tmp->next != &obj) {
                tmp = tmp->next;
            }
            tmp->next = obj.next;
        }
        obj.next = nullptr;
    }

    static T* const &get_head()
    {
        return head;
    }

    T* get(uint8_t index)
    {
        WITH_SEMAPHORE(sem);
        T *tmp = head;
        for (uint8_t i = 0; (i < index) && (tmp != nullptr); i++) {
            tmp = tmp->next;
        }
        return tmp;
    }

    // iterator
    class iterator
    {
        T *ptr;
        T *next;
    public:
        iterator(T *p) : ptr(p)
        {
            // record next as well this is so that if we remove the current element
            // we can still continue iterating
            if (ptr != nullptr) {
                next = ptr->next;
            } else {
                next = nullptr;
            }
        }
        iterator& operator++()
        {
            ptr = next;
            if (ptr != nullptr) {
                next = ptr->next;
            }
            return *this;
        }
        iterator operator++(int)
        {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        bool operator==(const iterator& rhs) const
        {
            return ptr == rhs.ptr;
        }
        bool operator!=(const iterator& rhs) const
        {
            return ptr != rhs.ptr;
        }
        T* operator*()
        {
            return ptr;
        }
    };

    static iterator begin()
    {
        return iterator(head);
    }
    static iterator end()
    {
        return iterator(nullptr);
    }
};

#define LIST_INIT(T) template<> T* dronecan::List<T>::head = nullptr; template<> dronecan::Semaphore dronecan::List<T>::sem = dronecan::Semaphore();

} // namespace DroneCAN
