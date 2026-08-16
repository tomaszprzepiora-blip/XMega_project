#ifndef XMEGA_MENU_H
#define XMEGA_MENU_H

#include "xmega_ui.h"

/*
 * Blocks (polling touch internally) until the user picks an item and it
 * finishes, or taps "Wyjdz" (exit). New settings/tools should be added as
 * additional entries in MENU_ITEMS in xmega_menu.c, not as one-off hidden
 * gestures - this menu is the place future features hang off of.
 */
void menu_run(const ui_driver_t *driver);

#endif
