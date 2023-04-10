#pragma once

#include "DataIface.h"
#include <ch.hpp>
namespace CubeFramework
{

class ShMemIface : public DataIface {
public:
    ShMemIface() = delete;
    ShMemIface(const ShMemIface&) = delete;
    ShMemIface& operator=(const ShMemIface&) = delete;

    ShMemIface(CanardIface &canard_iface) : DataIface(canard_iface) {
        if (_singleton) {
            chSysHalt("ShMemIface already instantiated");
        }
        _singleton = this;
    }

    void init();

    bool send(const CanardCANFrame &frame) override;
    void update_rx() override;
    static ShMemIface* get_singleton() { return _singleton; }
private:
    void signalI();

    // ring buffer
    class Buffer {
        friend class ShMemIface;
    public:
        Buffer(uint8_t *buffer, size_t size) : buffer(buffer), size(size) {}
        void reset() {
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

    class HWSemaphore {
    public:
        HWSemaphore();
        ~HWSemaphore();
    };

    uint8_t rx_thread_wa[512];
    thread_t *rx_thread;
    chibios_rt::EventSource _evt_src;

    static void rx_thread_trampoline(void *arg);
    static ShMemIface *_singleton;
};

} // namespace CubeFramework
