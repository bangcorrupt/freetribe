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
 * @file    app_menu.c
 *
 * @brief   Application menu state machine.
 *
 */

/*----- Includes -----------------------------------------------------*/

#include <stdbool.h>
#include <stdint.h>

#include "freetribe.h"

#include "app_main.h"

#include "micro_menu.h"

/*----- Macros and Definitions ---------------------------------------*/

/*----- Static variable definitions ----------------------------------*/

/*----- Extern variable definitions ----------------------------------*/

/*----- Static function prototypes -----------------------------------*/

static void _generic_print(const char *text);

/*----- Extern function implementations ------------------------------*/

/** Example menu item specific enter callback function, run when the associated
 * menu item is entered. */
static void Level1Item1_Enter(void) {}

/** Example menu item specific select callback function, run when the associated
 * menu item is selected. */
static void Level1Item1_Select(void) {}

MENU_ITEM(Menu_1, Menu_2, Menu_3, NULL_MENU, Menu_1_1, Level1Item1_Select,
          Level1Item1_Enter, "1. MAIN -> Sub A\n");

MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, NULL_MENU, NULL, NULL,
          "2. MAIN -> Sub B\n");
MENU_ITEM(Menu_3, Menu_1, Menu_2, NULL_MENU, NULL_MENU, NULL, NULL,
          "3. MAIN -> Sub C\n");

MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_2, Menu_1, NULL_MENU, NULL, NULL,
          "1.1 SUB_A -> Item 1\n");

MENU_ITEM(Menu_1_2, Menu_1_1, Menu_1_1, Menu_1, NULL_MENU, NULL, NULL,
          "1.2 SUB_A -> Item 2\n");

/* MENU_ITEM(Menu_1, Menu_2, Menu_3, NULL_MENU, Menu_1_1, Level1Item1_Select, */
/*           Level1Item1_Enter, NULL); */

/* MENU_ITEM(Menu_2, Menu_3, Menu_1, NULL_MENU, NULL_MENU, NULL, NULL, NULL); */
/* MENU_ITEM(Menu_3, Menu_1, Menu_2, NULL_MENU, NULL_MENU, NULL, NULL, NULL); */

/* MENU_ITEM(Menu_1_1, Menu_1_2, Menu_1_2, Menu_1, NULL_MENU, NULL, NULL, NULL);
 */

/* MENU_ITEM(Menu_1_2, Menu_1_1, Menu_1_1, Menu_1, NULL_MENU, NULL, NULL, NULL);
 */

void menu_init(void) {
    Menu_SetGenericWriteCallback(_generic_print);
    Menu_Navigate(&Menu_1);
}

void menu_up(void) { Menu_Navigate(MENU_PREVIOUS); }

void menu_down(void) { Menu_Navigate(MENU_NEXT); }

void menu_exit(void) { Menu_Navigate(MENU_PARENT); }

void menu_enter(void) {
    if (MENU_CHILD) {
        Menu_Navigate(MENU_CHILD);
    } else {
        Menu_EnterCurrentItem();
    }
}

/*----- Static function implementations ------------------------------*/

/* @brief       Generic function to write the text of a menu.
 *
 * @param text  String to print.
 *
 */
static void _generic_print(const char *text) {
    if (text) {
        print(text);
    }
}

/*----- End of file --------------------------------------------------*/
