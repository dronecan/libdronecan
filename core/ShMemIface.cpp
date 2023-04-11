#include "ShMemIface.h"
#include <hal.h>
#include <string.h>
#include "helpers.h"

using namespace CubeFramework;

#ifdef CORE_CM7
#define TX_HSEM_INDEX 0
#define RX_HSEM_INDEX 1
#else
#define TX_HSEM_INDEX 1
#define RX_HSEM_INDEX 0
#endif

void sem_free_it();

extern uint32_t __shared_mem_base__, __shared_mem_end__;
chibios_rt::EventSource ShMemIface::evt_src;
ShMemIface* ShMemIface::_singleton = nullptr;

void ShMemIface::init()
{
    volatile size_t shared_mem_size = uint32_t(&__shared_mem_end__) - uint32_t(&__shared_mem_base__);

#if defined(CORE_CM7)
    // use first half of shared memory for rx
    rx_buf = (Buffer*)&__shared_mem_base__;
    rx_buf->reset();
    rx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + sizeof(Buffer));
    rx_buf->size = (shared_mem_size/2) - sizeof(Buffer);

    // use second half of shared memory for tx
    tx_buf = (Buffer*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2));
    tx_buf->reset();
    tx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2) + sizeof(Buffer));
    tx_buf->size = (shared_mem_size/2) - sizeof(Buffer);
    memset(tx_buf->buffer, 0, tx_buf->size);

    // enable HSEM
    rccEnableAHB4(RCC_AHB4ENR_HSEMEN, true);
    // enable HSEM1 interrupt
    nvicEnableVector(HSEM1_IRQn, CORTEX_MAX_KERNEL_PRIORITY);
#else
    // use first half of shared memory for tx
    tx_buf = (Buffer*)&__shared_mem_base__;
    tx_buf->reset();
    tx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + sizeof(Buffer));
    tx_buf->size = (shared_mem_size/2) - sizeof(Buffer);
    memset(tx_buf->buffer, 0, tx_buf->size);

    // use second half of shared memory for rx
    rx_buf = (Buffer*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2));
    rx_buf->reset();
    rx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2) + sizeof(Buffer));
    rx_buf->size = (shared_mem_size/2) - sizeof(Buffer);

    // enable HSEM
    rccEnableAHB4(RCC_AHB4ENR_HSEMEN, true);
    // enable HSEM2 interrupt
    NVIC_SetPriority(HSEM2_IRQn, CORTEX_MAX_KERNEL_PRIORITY);
#endif

    // start rx thread
    rx_thread = chThdCreateStatic(rx_thread_wa, sizeof(rx_thread_wa), NORMALPRIO, rx_thread_trampoline, this);
}

void ShMemIface::rx_thread_trampoline(void *arg)
{
    ShMemIface *self = (ShMemIface *)arg;
    self->update_rx();
}

bool ShMemIface::send(const CanardCANFrame &frame)
{
    return tx_buf->push(frame);
}

void ShMemIface::signal_rx()
{
    chSysLockFromISR();
    evt_src.broadcastFlagsI(1U << RX_HSEM_INDEX);
    chSysUnlockFromISR();
}

void ShMemIface::update_rx()
{
    CanardCANFrame frame;
    chibios_rt::EventListener evt_listener;
    evt_src.registerMask(&evt_listener, 1U << RX_HSEM_INDEX);
    // set interrupt on RX_HSEM_INDEX
    HSEM_COMMON->IER |= (1U << RX_HSEM_INDEX);
    while (true) {
        // wait for semaphore
        chEvtWaitAnyTimeout(1U << RX_HSEM_INDEX, TIME_INFINITE);
        while (rx_buf->pop(frame)) {
            // canard_iface.handle_frame(frame, micros64());
        }
    }
}

// Ring buffer
#define FRAME_HEADER1 0x34
#define FRAME_HEADER2 0x12
#define FRAME_SIZE_WITHOUT_DATA 7

#define WITH_SEMAPHORE(index) JOIN( index, __COUNTER__ )

#define JOIN( index, counter ) _DO_JOIN( index, counter )
#define _DO_JOIN( index, counter ) ShMemIface::HWSemaphore _getsem ## counter(index)

ShMemIface::HWSemaphore::HWSemaphore(uint8_t i)
{
    if (i >= 32) {
        // invalid semaphore
        return;
    }
    volatile HSEM_TypeDef *hw_sem = HSEM;
    volatile HSEM_Common_TypeDef *hw_sem_core = HSEM_COMMON;
    hw_sem_core->IER |= (1U << i);
    chibios_rt::EventListener evt_listener;
    ShMemIface::evt_src.registerMask(&evt_listener, 1U << i);
    // try to get semaphore
    while (!(hw_sem->RLR[i] & HSEM_RLR_LOCK)) {
        // we don't hold the semaphore, sleep until its free
        chEvtWaitOne(1U << i);
    }
    // unregister event listener
    ShMemIface::evt_src.unregister(&evt_listener);
    // we have the semaphore clear interrupt
    hw_sem_core->IER &= ~(1U << i);

    // save unlock key
    unlock_key = HSEM->RLR[i] & 0xFFFF;
    sem_index = i;
}

ShMemIface::HWSemaphore::~HWSemaphore()
{
    HSEM->R[sem_index] = unlock_key;
}

size_t ShMemIface::Buffer::available() const
{
    if (head >= tail) {
        return size - head + tail;
    }
    return tail - head;
}

void ShMemIface::Buffer::advance(size_t bytes)
{
    if (bytes > available()) {
        // buffer is empty
        return;
    }
    tail += bytes;
    if (tail >= size) {
        tail -= size;
    }
}

size_t ShMemIface::Buffer::txspace() const
{
    return size - available();
}


void ShMemIface::Buffer::pushbyte(uint8_t byte)
{
    buffer[head++] = byte;
    if (head >= size) {
        head = 0;
    }
}

bool ShMemIface::Buffer::push(const CanardCANFrame &frame)
{
    WITH_SEMAPHORE(TX_HSEM_INDEX);
    if (txspace() > (size_t)(frame.data_len + FRAME_SIZE_WITHOUT_DATA)) {
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
    return true;
}

uint8_t ShMemIface::Buffer::peekbyte(size_t byte)
{
    if (tail + byte >= size) {
        return buffer[tail + byte - size];
    } else {
        return buffer[tail + byte];
    }
}

bool ShMemIface::Buffer::pop(CanardCANFrame &frame)
{
    WITH_SEMAPHORE(RX_HSEM_INDEX);
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

void sem_free_it()
{
    // send events to all listeners
    ShMemIface::evt_src.broadcastFlags(HSEM_COMMON->ISR);
    // clear interrupt flags
    HSEM_COMMON->ICR = HSEM_COMMON->ISR;
}

ShMemIface* shmem_iface()
{
    return ShMemIface::get_singleton();
}

#if defined(CORE_CM7)
CH_IRQ_HANDLER(Vector234);
CH_IRQ_HANDLER(Vector234)
{
    CH_IRQ_PROLOGUE();
    sem_free_it();
    CH_IRQ_EPILOGUE();
}
#else
CH_IRQ_HANDLER(Vector238);
CH_IRQ_HANDLER(Vector238)
{
    CH_IRQ_PROLOGUE();
    sem_free_it();
    CH_IRQ_EPILOGUE();
}
#endif
