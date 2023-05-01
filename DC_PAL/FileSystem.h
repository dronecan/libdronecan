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
class File
{
public:
    virtual ~File() = 0;
    virtual int read(void* buf, uint32_t size) = 0;
    virtual int write(const void* buf, uint32_t size) = 0;
    virtual int seek(uint32_t offset, int whence) = 0;
    virtual int flush() = 0;
    virtual int stat(const char *pathname, struct stat *stbuf) = 0;
};

class FileSystem
{
public:
    FileSystem(const char* root_path) : _root_path(root_path)
    {
        if (root_path == nullptr) {
#if defined(ARDUPILOT_BUILD)
            _root_path = "";
#else
            _root_path = ".";
#endif
        }
    }
    virtual File* open(const char* path, const char* mode) = 0;
    virtual void close(File* file) = 0;
    const char* root_path() const
    {
        return _root_path;
    }
private:
    const char* _root_path;
};

} // namespace DroneCAN
