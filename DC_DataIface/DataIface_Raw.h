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
#include "DataIface_Abstract.h"

namespace DroneCAN
{

class RawIface : public DataIface
{
protected:
    // ring buffer, this is not thread safe
    // thread safety is to be implemented in the derived class using this
    class Buffer
    {
        friend class RawIface;
    public:
        Buffer(uint8_t *buffer, size_t size) : buffer(buffer), size(size) {}
        void reset();
        bool push(const CanardCANFrame &frame);
        bool pop(CanardCANFrame &frame);
    private:
        // these are not thread safe
        // use only after taking semaphore
        size_t available() const;
        uint8_t peekbyte(size_t size);
        void pushbyte(uint8_t byte);
        void advance(size_t bytes);
        size_t txspace() const;
        uint8_t *buffer;
        size_t size;
        size_t head;
        size_t tail;
    } *rx_buf, *tx_buf;
public:
    static constexpr uint8_t FRAME_HEADER1 = 0x34;
    static constexpr uint8_t FRAME_HEADER2 = 0x12;
    static constexpr uint8_t FRAME_SIZE_WITHOUT_DATA = 7;
};

} // namespace DroneCAN
