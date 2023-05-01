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

#if defined(LINUX_BUILD)

#ifndef DRONECAN_HAS_FILESYSTEM
#define DRONECAN_HAS_FILESYSTEM
#endif

#include "FileSystem_Abstract.h"
namespace DroneCAN
{

class PosixFS : public FileSystem
{
public:
    PosixFS(const char* root_path);

    virtual File* open(const char* path, const char* mode) override;

    virtual void close(File* file) override;

    static PosixFS* instance()
    {
        return _instance;
    }
private:
    static PosixFS* _instance;
};

class PosixFile : public File
{
    friend class PosixFS;
public:
    PosixFile(const char* path, const char* mode);
    ~PosixFile();
    int read(void* buf, uint32_t size) override;
    int write(const void* buf, uint32_t size) override;
    int seek(uint32_t offset, int whence) override;
    int flush() override;
    int stat(const char *pathname, struct stat *stbuf) override;
private:
    int _fd;
};

} // namespace DroneCAN
#endif // LINUX_BUILD
