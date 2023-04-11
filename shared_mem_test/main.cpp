#include <ch.h>
#include <hal.h>
#include <core/helpers.h>
#include <core/ShMemIface.h>

#ifdef CORE_CM7
#include "usbcfg.h"
#include "chprintf.h"
#define print(s, ...) chprintf(STDOUT, s, ##__VA_ARGS__)
#else
#define print(s, ...)
#endif

CubeFramework::ShMemIface shmem;

int main(void)
{
    CanardCANFrame frame;
    halInit();
    chSysInit();
    shmem.init();

    /*
    * Initializes a serial-over-USB CDC driver.
    */
#ifdef CORE_CM7
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1500);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);
#endif
    // setup dummy frame
    frame.id = 0x123;
    frame.data_len = 8;
    for (uint8_t i=0; i<8; i++) {
        frame.data[i] = i;
    }

    print("Starting Shared Memory Test\n");
    while (1) {
        frame.id++;
        // send frame
        shmem.send(frame);
        chThdSleepMilliseconds(500);
    }
}
