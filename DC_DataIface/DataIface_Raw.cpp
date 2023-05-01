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

#include "DataIface_Raw.h"

using namespace DroneCAN;

size_t RawIface::Buffer::available() const
{
    return (head >= tail) ? (head - tail) : (size - tail + head);
}

void RawIface::Buffer::advance(size_t bytes)
{
    tail = (bytes + tail) % size;
}

size_t RawIface::Buffer::txspace() const
{
    return size - available();
}

void RawIface::Buffer::pushbyte(uint8_t byte)
{
    buffer[head++] = byte;
    if (head >= size) {
        head = 0;
    }
}

// only done by transmitter
void RawIface::Buffer::reset()
{
    head = 0;
    tail = 0;
}

bool RawIface::Buffer::push(const CanardCANFrame &frame)
{
    if (txspace() < (size_t)(frame.data_len + FRAME_SIZE_WITHOUT_DATA)) {
        // buffer is full
        return false;
    }
    // place 32 bit ID
    pushbyte(FRAME_HEADER1);
    pushbyte(frame.id & 0xFF);
    pushbyte((frame.id >> 8) & 0xFF);
    pushbyte((frame.id >> 16) & 0xFF);
    pushbyte((frame.id >> 24) & 0xFF);
    // place 8 bit data length
    pushbyte(frame.data_len);
    // place data
    for (size_t i = 0; i < frame.data_len; i++) {
        pushbyte(frame.data[i]);
    }
    return true;
}

uint8_t RawIface::Buffer::peekbyte(size_t byte)
{
    return buffer[(tail + byte) % size];
}

bool RawIface::Buffer::pop(CanardCANFrame &frame)
{
    if (available() == 0) {
        // buffer is empty
        return false;
    }
    // peek 32 bit header
    if (peekbyte(0) != FRAME_HEADER1) {
        return false;
    }
    frame.id = peekbyte(1);
    frame.id |= (uint32_t)peekbyte(2) << 8;
    frame.id |= (uint32_t)peekbyte(3) << 16;
    frame.id |= (uint32_t)peekbyte(4) << 24;
    frame.data_len = peekbyte(5);
    // get data
    for (size_t i = 0; i < frame.data_len; i++) {
        frame.data[i] = peekbyte(6 + i);
    }
    // advance
    advance(frame.data_len + FRAME_SIZE_WITHOUT_DATA);
    return true;
}
