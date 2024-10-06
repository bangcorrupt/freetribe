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
 * @file    init.h
 *
 * @brief   Public API for Blackfin initialisation.
 *
 */

#ifndef BFIN_INIT_H
#define BFIN_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

/*----- Includes -----------------------------------------------------*/

#include <blackfin.h>

/*----- Macros -------------------------------------------------------*/

/*----- Extern variable declarations ---------------------------------*/

void pll_init(void);
void ebiu_init(void);
void sysint_init(void);
void gpio_init(void);
void dma_init(void);

/*----- Extern function prototypes -----------------------------------*/

#ifdef __cplusplus
}
#endif
#endif /* BFIN_INIT_H */

/*----- End of file --------------------------------------------------*/
