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

#include "DataIface_LogFile.h"
#if defined(DRONECAN_HAS_FILESYSTEM)
#include <hal.h>
#include <string.h>
#include "helpers.h"
#include "DataIface_Raw.h"

using namespace DroneCAN;

bool LogFileIface::init()
{
    if (read_file == nullptr || write_file == nullptr) {
        return false;
    }
    return true;
}

uint32_t LogFileIface::crc32(const uint8_t *data, size_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < len; i++) {
        uint8_t byte = data[i];
        crc ^= byte;
        for (size_t j = 0; j < 8; j++) {
            uint32_t mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

bool LogFileIface::send(const CanardCANFrame &frame)
{
    // write to file
    if (!write_file) {
        return false;
    }

    if (write_file->write(&RawIface::FRAME_HEADER1, 1) != 1) {
        return false;
    }
    if (write_file->write(&frame.id, 4) != 4) {
        return false;
    }
    if (write_file->write(&frame.data_len, 1) != 1) {
        return false;
    }
    if (write_file->write(frame.data, frame.data_len) != frame.data_len) {
        return false;
    }
    uint32_t crc = crc32(frame.data, frame.data_len);
    if (write_file->write(&crc, 4) != 4) {
        return false;
    }
    return true;
}

void LogFileIface::update_rx()
{
    // read from file
    if (!read_file) {
        return;
    }

    CanardCANFrame frame;
    uint8_t header;

    // find header
    while (true) {
        if (read_file->read(&header, 1) != 1) {
            return;
        }
        if (header == RawIface::FRAME_HEADER1) {
            break;
        }
    }
    if (read_file->read(&frame.id, 4) != 4) {
        return;
    }
    if (read_file->read(&frame.data_len, 1) != 1) {
        return;
    }
    if (read_file->read(frame.data, frame.data_len) != frame.data_len) {
        return;
    }
    uint32_t crc;
    if (read_file->read(&crc, 4) != 4) {
        return;
    }
    if (crc != crc32(frame.data, frame.data_len)) {
        return;
    }
    rx_cbs(frame);
}

#endif // DRONECAN_HAS_FILESYSTEM
