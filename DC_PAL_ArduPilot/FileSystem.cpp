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
#include "FileSystem_ArduPilot.h"
#include <AP_FileSystem/AP_FileSystem.h>

using namespace dronecan::ardupilot;

FileSystem* FileSystem::_instance = nullptr;

FileSystem::FileSystem(const char* root_path) :
    dronecan::FileSystem(root_path)
{
    if (_instance != nullptr) {
        dronecan_assert(false, "ArduPilotFS already instantiated");
    }
    _instance = this;
}

File* FileSystem::open(const char* path, const char* mode)
{
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", root_path(), path);
    return new ArduPilotFile(full_path, mode);
}

void FileSystem::close(File* file)
{
    delete file;
}

File::File(const char* path, const char* mode)
{
    _fd = AP::FS().open(path, mode);
    dronecan_assert(_fd != -1, "Failed to open file %s", path);
}

File::~File()
{
    AP::FS().close(_fd);
}

int File::read(void* buf, uint32_t size)
{
    return AP::FS().read(_fd, buf, size);
}

int File::write(const void* buf, uint32_t size)
{
    return AP::FS().write(_fd, buf, size);
}

int File::seek(uint32_t offset, int whence)
{
    return AP::FS().lseek(_fd, offset, whence);
}

int File::flush()
{
    return AP::FS().fsync(_fd);
}

int File::stat(const char *pathname, struct stat *stbuf);
{
    return AP::FS().stat(pathname, stbuf);
}
