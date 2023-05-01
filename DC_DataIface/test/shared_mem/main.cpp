#include <ch.h>
#include <hal.h>
#include <dronecan/helpers.h>
#include <dronecan/DataIfaceShMem.h>


DroneCAN::ShMemIface shmem;

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
    while (1) {
        chThdSleepMilliseconds(50);
    }
}
