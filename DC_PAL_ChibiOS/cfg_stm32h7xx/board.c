/*
    ChibiOS - Copyright (C) 2006..2020 Giovanni Di Sirio

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

/*
 * This file has been automatically generated using ChibiStudio board
 * generator plugin. Do not edit manually.
 */

#include "hal.h"
#include "stm32_gpio.h"
#include "usbcfg.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Early initialization code.
 * @details GPIO ports and system clocks are initialized before everything
 *          else.
 */
void __early_init(void)
{
    stm32_clock_init();
    // set first 16 bits of BOOT address
    if ((FLASH->BOOT4_CUR & 0x081FLU) != 0x081FLU) {
        //unlock flash
        if (FLASH->OPTCR & FLASH_OPTCR_OPTLOCK) {
            /* Unlock sequence */
            FLASH->OPTKEYR = 0x08192A3B;
            FLASH->OPTKEYR = 0x4C5D6E7F;
        }
        while (FLASH->OPTSR_CUR & FLASH_OPTSR_OPT_BUSY) {
        }
        FLASH->BOOT4_PRG = (FLASH->BOOT4_CUR & 0xFFFF0000LU) | 0x081FU;
        // enable CM4 boot as well, most likely its already enabled
        FLASH->OPTSR_PRG |= FLASH_OPTSR_BCM4;
        // start programming
        FLASH->OPTCR |= FLASH_OPTCR_OPTSTART;
        // wait for completion by checking busy bit
        while (FLASH->OPTSR_CUR & FLASH_OPTSR_OPT_BUSY) {
        }
        // lock flash
        FLASH->OPTCR |= FLASH_OPTCR_OPTLOCK;
        while (FLASH->OPTSR_CUR & FLASH_OPTSR_OPT_BUSY) {
        }
        NVIC_SystemReset();
    }
}

#if HAL_USE_SDC || defined(__DOXYGEN__)
/**
 * @brief   SDC card detection.
 */
bool sdc_lld_is_card_inserted(SDCDriver *sdcp)
{

    (void)sdcp;
    /* CHTODO: Fill the implementation.*/
    return true;
}

/**
 * @brief   SDC card write protection detection.
 */
bool sdc_lld_is_write_protected(SDCDriver *sdcp)
{

    (void)sdcp;
    /* CHTODO: Fill the implementation.*/
    return false;
}
#endif /* HAL_USE_SDC */

#if HAL_USE_MMC_SPI || defined(__DOXYGEN__)
/**
 * @brief   MMC_SPI card detection.
 */
bool mmc_lld_is_card_inserted(MMCDriver *mmcp)
{

    (void)mmcp;
    /* CHTODO: Fill the implementation.*/
    return true;
}

/**
 * @brief   MMC_SPI card write protection detection.
 */
bool mmc_lld_is_write_protected(MMCDriver *mmcp)
{

    (void)mmcp;
    /* CHTODO: Fill the implementation.*/
    return false;
}
#endif

/**
 * @brief   Board-specific initialization code.
 * @note    You can add your board-specific code here.
 */
void boardInit(void)
{

}
