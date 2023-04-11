#include <ch.h>
#include <hal.h>

int main(void)
{
    halInit();
    chSysInit();
#ifdef CORE_M7
    /*
    * Initializes a serial-over-USB CDC driver.
    */
    sduObjectInit(&PORTAB_SDU1);
    sduStart(&PORTAB_SDU1, &serusbcfg);

    /*
    * Activates the USB driver and then the USB bus pull-up on D+.
    * Note, a delay is inserted in order to not have to disconnect the cable
    * after a reset.
    */
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1500);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);
#endif

    while (1) {
        chThdSleepMilliseconds(500);
    }
}
