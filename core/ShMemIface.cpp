#include "ShMemIface.h"
#include <hal.h>

using namespace CubeFramework;

thread_t ShMemIface::thread_rx;

void sem_free_it();

void ShMemIface::init() {
    const size_t shared_mem_size = __shared_mem_end__ - __shared_mem_base__;

#if defined(CORE_CM7)
    // use first half of shared memory for rx
    rx_buf = __shared_mem_base__;
    memset(rx_buf, 0, sizeof(Buffer));
    rx_buf->buffer = __shared_mem_base__ + sizeof(Buffer);
    rx_buf->size = (shared_mem_size/2) - sizeof(Buffer);

    // use second half of shared memory for tx
    tx_buf = __shared_mem_base__ + (shared_mem_size/2);
    memset(tx_buf, 0, sizeof(Buffer));
    tx_buf->buffer = __shared_mem_base__ + (shared_mem_size/2) + sizeof(Buffer);
    tx_buf->size = (shared_mem_size/2) - sizeof(Buffer);

    // enable HSEM
    rccEnableAHB4(RCC_AHB4ENR_HSEMEN, true);
    // enable HSEM1 interrupt
    nvicEnableVector(HSEM1_IRQn, CORTEX_MAX_KERNEL_PRIORITY);
#else
    // use first half of shared memory for tx
    tx_buf = __shared_mem_base__;
    memset(tx_buf, 0, sizeof(Buffer));
    tx_buf->buffer = __shared_mem_base__ + sizeof(Buffer);
    tx_buf->size = (shared_mem_size/2) - sizeof(Buffer);

    // use second half of shared memory for rx
    rx_buf = __shared_mem_base__ + (shared_mem_size/2);
    memset(rx_buf, 0, sizeof(Buffer));
    rx_buf->buffer = __shared_mem_base__ + (shared_mem_size/2) + sizeof(Buffer);
    rx_buf->size = (shared_mem_size/2) - sizeof(Buffer);

    // enable HSEM
    rccEnableAHB4(RCC_AHB4ENR_HSEMEN, true);
    // enable HSEM2 interrupt
    NVIC_SetPriority(HSEM2_IRQn, CORTEX_MAX_KERNEL_PRIORITY);
#endif

    // start rx thread
    rx_thread = chThdCreateStatic(rx_thread_wa, sizeof(rx_thread_wa), NORMALPRIO, rx_thread_trampoline, this);
}

void ShMemIface::rx_thread_trampoline(void *arg) {
    ShMemIface *self = (ShMemIface *)arg;
    self->update_rx();
}

bool ShMemIface::send(const CanardCANFrame &transfer) {
    return tx_buf->push(frame);
}

void ShMemIface::signalI()
{
    chSysLockFromISR();
    ch_evt_src_.broadcastFlagsI(0x1);
    chSysUnlockFromISR();
}

void ShMemIface::update_rx() {
    CanardCANFrame frame;
    chibios_rt::EventListener evt_listener;
    ch_evt_src_.registerMask(&evt_listener, 0x1);

    while (true) {
        // wait for semaphore
        chEvtWaitAnyTimeout(0x1, TIME_INFINITE);
        while (rx_buf->pop(frame)) {
            canard_iface.handle_frame(frame);
        }
    }
}

// Ring buffer
#define FRAME_HEADER1 0x34
#define FRAME_HEADER2 0x12
#define FRAME_SIZE_WITHOUT_DATA 7

#define WITH_SEMAPHORE() JOIN( __COUNTER__ )

#define JOIN( counter ) _DO_JOIN( counter )
#define _DO_JOIN( counter ) ShMemIface::HWSemaphore _getsem ## counter()

size_t ShMemIface::Buffer::available() const {
    if (head >= tail) {
        return size - head + tail;
    }
    return tail - head;
}

void ShMemIface::Buffer::advance(size_t bytes) {
    if (bytes > available()) {
        // buffer is empty
        return;
    }
    tail += bytes;
    if (tail >= size) {
        tail -= size;
    }
}

size_t ShMemIface::Buffer::txspace() const {
    return size - available();
}


void ShMemIface::Buffer::pushbyte(uint8_t byte) {
    buffer[head++] = byte;
    if (head >= size) {
        head = 0;
    }
}

bool ShMemIface::Buffer::push(const CanardCANFrame &frame) {
    WITH_SEMAPHORE();
    if (txspace() > (frame.data_len + FRAME_SIZE_WITHOUT_DATA)) {
        // buffer is full
        return false;
    }
    // push 32 bit header
    pushbyte(FRAME_HEADER1);
    pushbyte(FRAME_HEADER2);
    // place 32 bit ID
    pushbyte(frame.id & 0xFF);
    pushbyte((frame.id >> 8) & 0xFF);
    pushbyte((frame.id >> 16) & 0xFF);
    pushbyte((frame.id >> 24) & 0xFF);
    // place 8 bit DLC
    pushbyte(frame.data_len);
    // place data
    for (size_t i = 0; i < frame.data_len; i++) {
        pushbyte(frame.data[i]);
    }
}

uint8_t ShMemIface::Buffer::peekbyte(uint8_t byte) {
    if (tail + byte >= size) {
        return buffer[tail + byte - size];
    } else {
        return buffer[tail + byte];
    }
}

bool ShMemIface::Buffer::pop(CanardCANFrame &frame) {
    WITH_SEMAPHORE();
    if (available() == 0) {
        // buffer is empty
        return false;
    }
    // peek 32 bit header
    if (peekbyte(0) != FRAME_HEADER1) {
        return false;
    }
    if (peekbyte(1) != FRAME_HEADER2) {
        return false;
    }
    // place 32 bit ID
    frame.id = peekbyte(2);
    frame.id |= peekbyte(3) << 8;
    frame.id |= peekbyte(4) << 16;
    frame.id |= peekbyte(5) << 24;
    // place 8 bit DLC
    frame.data_len = peekbyte(6);
    // place data
    for (size_t i = 0; i < frame.data_len; i++) {
        frame.data[i] = peekbyte(7+i);
    }
    // advance
    advance(frame.data_len + FRAME_SIZE_WITHOUT_DATA);
    return true;
}

void sem_free_it() {
}

ShMemIface* shmem_iface() {
    return ShMemIface::get_singleton();
}

#if defined(CORE_CM7)
CH_IRQ_HANDLER(Vector234);
CH_IRQ_HANDLER(Vector234) {
    CH_IRQ_PROLOGUE();
    sem_free_it();
    CH_IRQ_EPILOGUE();
}
#else
CH_IRQ_HANDLER(Vector238);
CH_IRQ_HANDLER(Vector238) {
    CH_IRQ_PROLOGUE();
    sem_free_it();
    CH_IRQ_EPILOGUE();
}
#endif
