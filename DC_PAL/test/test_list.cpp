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

#include <DC_PAL/List.h>
#include <gtest/gtest.h>

using namespace dronecan;

class TestObject : public List<TestObject>
{
public:
    TestObject() : m_value(0) {}
    TestObject(int value) : m_value(value) {}
    int m_value;
};

LIST_INIT(TestObject);

TEST(List, Basic)
{
    TestObject obj1(1), obj2(2), obj3(3);
    const auto list = TestObject::get_head();

    ASSERT_EQ(list->m_value, 1);
    ASSERT_EQ(list->get(0)->m_value, 1);
    ASSERT_EQ(list->get(1)->m_value, 2);
    ASSERT_EQ(list->get(2)->m_value, 3);
    ASSERT_EQ(list->get(3), nullptr);

    list->remove(obj2);
    ASSERT_EQ(list->m_value, 1);
    ASSERT_EQ(list->get(0)->m_value, 1);
    ASSERT_EQ(list->get(1)->m_value, 3);
    ASSERT_EQ(list->get(2), nullptr);

    for (auto obj : *list) {
        obj->m_value = 7;
    }

    ASSERT_EQ(list->m_value, 7);
    ASSERT_EQ(list->get(0)->m_value, 7);
    ASSERT_EQ(list->get(1)->m_value, 7);
    ASSERT_EQ(list->get(2), nullptr);

    list->remove(obj1);

    for (auto obj : *list) {
        obj->m_value = 8;
    }

    ASSERT_EQ(list->m_value, 7); // we have removed obj1 from the list, so it should not be modified
    ASSERT_EQ(list->get(0)->m_value, 8);
    ASSERT_EQ(list->get(1), nullptr);

    list->remove(obj3);

    for (auto obj : *list) {
        obj->m_value = 9;
    }

    ASSERT_EQ(obj3.m_value, 8); // we have removed obj3 from the list, so it should not be modified
    ASSERT_EQ(list->get(0), nullptr);
}
