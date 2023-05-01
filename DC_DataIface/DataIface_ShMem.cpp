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

#if defined(CHIBIOS_BUILD)

#include "DataIface_ShMem.h"

#include <hal.h>
#include <string.h>
#include "helpers.h"

using namespace DroneCAN;

#ifdef CORE_CM7
#define TX_HSEM_INDEX 0
#define RX_HSEM_INDEX 1
#define HSEM_CORE_ID  0x3
#else
#define TX_HSEM_INDEX 1
#define RX_HSEM_INDEX 0
#define HSEM_CORE_ID  0x1
#endif

void sem_free_it();

extern uint32_t __shared_mem_base__, __shared_mem_end__;
chibios_rt::EventSource ShMemIface::evt_src;
ShMemIface* ShMemIface::_singleton = nullptr;

bool ShMemIface::init()
{
    volatile size_t shared_mem_size = uint32_t(&__shared_mem_end__) - uint32_t(&__shared_mem_base__);

    // enable HSEM
    rccEnableAHB4(RCC_AHB4ENR_HSEMEN, true);

#if defined(CORE_CM7)
    // enable HSEM1 interrupt
    nvicEnableVector(HSEM1_IRQn, CORTEX_MAX_KERNEL_PRIORITY);

    {
        WITH_SEMAPHORE(RX_HSEM_INDEX);
        // use first half of shared memory for rx
        rx_buf = (Buffer*)&__shared_mem_base__;
        rx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + sizeof(Buffer));
        rx_buf->size = (shared_mem_size/2) - sizeof(Buffer);
    }

    // use second half of shared memory for tx
    {
        WITH_SEMAPHORE(TX_HSEM_INDEX);
        tx_buf = (Buffer*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2));
        tx_buf->reset();
        tx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2) + sizeof(Buffer));
        tx_buf->size = (shared_mem_size/2) - sizeof(Buffer);
        memset(tx_buf->buffer, 0, tx_buf->size);
    }
#else
    // enable HSEM2 interrupt
    NVIC_SetPriority(HSEM2_IRQn, CORTEX_MAX_KERNEL_PRIORITY);

    {
        WITH_SEMAPHORE(TX_HSEM_INDEX);
        // use first half of shared memory for tx
        tx_buf = (Buffer*)&__shared_mem_base__;
        tx_buf->reset();
        tx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + sizeof(Buffer));
        tx_buf->size = (shared_mem_size/2) - sizeof(Buffer);
        memset(tx_buf->buffer, 0, tx_buf->size);
    }

    {
        WITH_SEMAPHORE(RX_HSEM_INDEX);
        // use second half of shared memory for rx
        rx_buf = (Buffer*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2));
        rx_buf->buffer = (uint8_t*)(uint32_t(&__shared_mem_base__) + (shared_mem_size/2) + sizeof(Buffer));
        rx_buf->size = (shared_mem_size/2) - sizeof(Buffer);
    }
#endif

    // start rx thread
    rx_thread = chThdCreateStatic(rx_thread_wa, sizeof(rx_thread_wa), NORMALPRIO, rx_thread_trampoline, this);
    if (rx_thread == nullptr) {
        return false;
    }
    return true;
}

void ShMemIface::rx_thread_trampoline(void *arg)
{
    ShMemIface *self = (ShMemIface *)arg;
    self->update_rx();
}

bool ShMemIface::send(const CanardCANFrame &frame)
{
    WITH_SEMAPHORE(TX_HSEM_INDEX);
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
    while (true) {
        // set interrupt on RX_HSEM_INDEX
        HSEM_COMMON->IER |= (1U << RX_HSEM_INDEX);
        // wait for semaphore
        chEvtWaitAnyTimeout(1U << RX_HSEM_INDEX, TIME_INFINITE);
        // disable interrupt on RX_HSEM_INDEX while we read the buffer
        HSEM_COMMON->IER &= ~(1U << RX_HSEM_INDEX);
        while (true) {
            {
                WITH_SEMAPHORE(RX_HSEM_INDEX);
                if (!rx_buf->pop(frame)) {
                    break;
                }
            }
            // call rx callback
            rx_cbs(frame);
        }
    }
}


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
    while ((!(hw_sem->RLR[i] & HSEM_RLR_LOCK)) || ((hw_sem->RLR[i] >> 8) & 0xFF) != HSEM_CORE_ID) {
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
    // clear our interrupt
    HSEM_COMMON->ICR |= (1U << sem_index);
}

size_t ShMemIface::Buffer::available() const
{
    return (head >= tail) ? (head - tail) : (size - tail + head);
}

void ShMemIface::Buffer::advance(size_t bytes)
{
    tail = (bytes + tail) % size;
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

void sem_free_it()
{
    // send events to all listeners
    ShMemIface::evt_src.broadcastFlagsI(HSEM_COMMON->ISR);
    // clear all interrupt flags
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

#endif // CHIBIOS_BUILD