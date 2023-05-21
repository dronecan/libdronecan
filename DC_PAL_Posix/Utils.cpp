#include "Utils.h"
#include <stdint.h>
#include <time.h>

namespace dronecan
{
// micros64
uint64_t micros64()
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
}

} // namespace dronecan
