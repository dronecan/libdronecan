#include "hal.h"
#include "helpers.h"


uint32_t DroneCAN::micros32()
{
    systime_t time = chVTGetSystemTimeX();
    time *= 1000000/CH_CFG_ST_FREQUENCY;
    return time;
}

uint64_t DroneCAN::micros64()
{
    static uint32_t last_time = 0;
    static uint32_t overflow_count = 0;
    uint32_t time = micros32();
    if (time < last_time) {
        overflow_count++;
    }
    last_time = time;
    return ((uint64_t)overflow_count << 32) | (uint64_t)time;
}
