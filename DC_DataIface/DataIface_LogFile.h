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
#include "FileSystem.h"
#if defined(DRONECAN_HAS_FILESYSTEM)
#include "DataIface_Abstract.h"
#include "helpers.h"

namespace DroneCAN
{

class LogFileIface : public DataIface
{
public:
    LogFileIface(File *_read_file, File *_write_file) :
        read_file(_read_file),
        write_file(_write_file)
    {
        if (singleton != nullptr) {
            dronecan_assert(true, "LogFileIface already instantiated");
        }
        singleton = this;
    }
    bool init() override;
    bool send(const CanardCANFrame &frame) override;
    void update_rx() override;
    uint32_t crc32(const uint8_t *data, size_t len);

    static LogFileIface* get_singleton()
    {
        return singleton;
    }

private:
    static LogFileIface *singleton;
    File *read_file;
    File *write_file;
};

} // namespace DroneCAN

#endif // DRONECAN_HAS_FILESYSTEM
