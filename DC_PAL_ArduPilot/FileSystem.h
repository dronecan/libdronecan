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

#include <DC_PAL/FileSystem.h>

namespace dronecan::ardupilot
{

class FileSystem : public dronecan::FileSystem
{
public:
    FileSystem(const char* root_path);

    File* open(const char* path, const char* mode) override;

    static FileSystem* instance()
    {
        return _instance;
    }
private:
    static FileSystem* _instance;
};

class File : public dronecan::File
{
    friend class ArduPilotFS;
public:
    File(const char* path, const char* mode);
    ~File();
    int read(void* buf, uint32_t size) override;
    int write(const void* buf, uint32_t size) override;
    int seek(uint32_t offset, int whence) override;
    int flush() override;
    int stat(const char *pathname, struct stat *stbuf) override;
private:
    int _fd;
};
} // namespace ArduPilot
