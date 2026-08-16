/**************************************************************************************************
* File: TFT_Test.c
* Company: (c) mikroElektronika, 2011
* Revision History:
*     20111028 (JK);
*       - revision;
* Description:
*       This is simple calculator example.
* Target:
*       MCU:             P32MX460F512L
*       Dev.Board:       Mikromedia_for_PIC32
*                        http://www.mikroe.com/eng/products/view/595/mikromedia-for-pic32/
*       Modules:         TFT 240x320 Colour display
*                        ac:Touch_Panel
*       Oscillator:      80000000 Hz (80MHz)
*       SW:              mikroC PRO for PIC32
*                        http://www.mikroe.com/eng/products/view/623/mikroc-pro-for-pic32/
* NOTES:
************************************************************************************************/

#include "Calculator_objects.h"

void main() {

  Start_TP();

  while (1) {
    Check_TP();

  }

}