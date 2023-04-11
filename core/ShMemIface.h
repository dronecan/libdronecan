#pragma once

#include "DataIface.h"
#include <ch.hpp>
namespace CubeFramework
{

class ShMemIface : public DataIface
{
public:
    ShMemIface(const ShMemIface&) = delete;
    ShMemIface& operator=(const ShMemIface&) = delete;

    ShMemIface()
    {
        if (_singleton) {
            chSysHalt("ShMemIface already instantiated");
        }
        _singleton = this;
    }

    void init();

    bool send(const CanardCANFrame &frame) override;
    void update_rx() override;
    static ShMemIface* get_singleton()
    {
        return _singleton;
    }
    static chibios_rt::EventSource evt_src;
private:
    void signal_rx();

    // ring buffer
    class Buffer
    {
        friend class ShMemIface;
    public:
        Buffer(uint8_t *buffer, size_t size) : buffer(buffer), size(size) {}
        void reset()
        {
            head = 0;
            tail = 0;
        }
        bool push(const CanardCANFrame &frame);
        bool pop(CanardCANFrame &frame);
    private:
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

    class HWSemaphore
    {
    public:
        HWSemaphore(uint8_t i);
        ~HWSemaphore();
    private:
        uint16_t unlock_key;
        uint8_t sem_index;
    };

    uint8_t rx_thread_wa[512];
    thread_t *rx_thread;

    static void rx_thread_trampoline(void *arg);
    static ShMemIface *_singleton;
};

} // namespace CubeFramework
