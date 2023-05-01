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

#include <DC_PAL/FileSystem.h>

namespace dronecan::empty
{

class File : public dronecan::File
{
public:
    ~File() override {}
    int read(void* buf, uint32_t size) override
    {
        return 0;
    }
    int write(const void* buf, uint32_t size) override
    {
        return 0;
    }
    int seek(uint32_t offset, int whence) override
    {
        return 0;
    }
    int flush() override
    {
        return 0;
    }
    int stat(const char *pathname, struct stat *stbuf) override
    {
        return 0;
    }
};

class FileSystem : public dronecan::FileSystem
{
public:
    FileSystem() : dronecan::FileSystem("") {}
    File* open(const char* path, const char* mode) override
    {
        return nullptr;
    }
    void close(dronecan::File* file) override
    {
    }
};
} // namespace dronecan::empty
