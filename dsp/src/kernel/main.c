/*----------------------------------------------------------------------

                     This file is part of Freetribe

                https://github.com/bangcorrupt/freetribe

                                License

                   GNU AFFERO GENERAL PUBLIC LICENSE
                      Version 3, 19 November 2007

                           AGPL-3.0-or-later

 Freetribe is free software: you can redistribute it and/or modify it
under the terms of the GNU Affero General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
                  (at your option) any later version.

     Freetribe is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty
        of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
          See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.

                       Copyright bangcorrupt 2023

----------------------------------------------------------------------*/

/*
 * @file    main.c
 *
 * @brief   Main function for Blackfin firmware.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <blackfin.h>
#include <builtins.h>

#include "init.h"
#include "module.h"
#include "per_gpio.h"
#include "per_spi.h"
#include "per_sport.h"
#include "svc_cpu.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

/*----- Extern function implementations ------------------------------*/

int main(void) {

    *pPORTGIO_SET = HWAIT;

    // TODO: Move to initcode, before main.
    pll_init();
    ebiu_init();

    per_gpio_init();

    sysint_init();

    dma_init();

    spi_init();

    sport0_init();
    // sport1_init();

    module_init();

    while (1) {

        if (sport0_frame_received()) {
            // TODO: Disable interrupts while processing audio.
            module_process(sport0_get_rx_buffer(), sport0_get_tx_buffer());

            sport0_frame_processed();
        }

        svc_cpu_task();
    }
}

void sysint_init(void) {

    // Clear interrupt assignment registers.
    *pSIC_IAR0 = 0;
    *pSIC_IAR1 = 0;
    *pSIC_IAR2 = 0;
    *pSIC_IAR3 = 0;
    *pSIC_IAR4 = 0;
    *pSIC_IAR5 = 0;
    *pSIC_IAR6 = 0;
    *pSIC_IAR7 = 0;

    *pSIC_IWR0 = 0xffffffff;
    *pSIC_IWR1 = 0xffffffff;
}

/*----- Static function implementations ------------------------------*/

/*----- End of file --------------------------------------------------*/
