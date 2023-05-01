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

#if defined(LINUX_BUILD)

#include "FileSystem_Posix.h"
#include "helpers.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

using namespace DroneCAN;


PosixFS* PosixFS::_instance = nullptr;

PosixFS::PosixFS(const char *root_path) :
    FileSystem(root_path)
{
    if (_instance != nullptr) {
        dronecan_assert(false, "FileSystemPosix already instantiated");
    }
    _instance = this;
}

File* PosixFS::open(const char *path, const char *mode)
{
    PosixFile *file = (PosixFile*)dronecan::allocate(sizeof(PosixFile));
    if (file == nullptr) {
        return nullptr;
    }
    new (file) PosixFile(path, mode);
    if (file->_fd == -1) {
        delete file;
        return nullptr;
    }
}

void PosixFS::close(File *file)
{
    delete file;
}

PosixFile::PosixFile(const char *path, const char *mode)
{
    _fd = ::open(path, O_RDWR | O_CREAT, 0777);
}

int PosixFile::read(void *buf, uint32_t size)
{
    return ::read(_fd, buf, size);
}

int PosixFile::write(const void *buf, uint32_t size)
{
    return ::write(_fd, buf, size);
}

int PosixFile::seek(uint32_t offset, int whence)
{
    return ::lseek(_fd, offset, whence);
}

int PosixFile::flush()
{
    return ::fsync(_fd);
}

int PosixFile::stat(const char *pathname, struct stat *stbuf)
{
    return ::stat(pathname, stbuf);
}

PosixFile::~PosixFile()
{
    ::close(_fd);
}

#endif // LINUX_BUILD
