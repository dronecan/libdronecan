#include <stdint.h>

#ifdef CORE_CM7
#include "usbcfg.h"
#include "chprintf.h"
#define print(s, ...) chprintf(STDOUT, s, ##__VA_ARGS__)
#else
#define print(s, ...)
#endif

namespace DroneCAN
{

uint32_t micros32();
uint64_t micros64();

} // namespace DroneCAN
