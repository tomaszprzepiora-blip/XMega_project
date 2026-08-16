/*******************************************************************************************
* Project name:
*  UART (Simple usage of UART module library functions)
* Copyright:
* (c) Mikroelektronika, 2010.
* Revision History:
*   20110513:
*     - initial release (FJ);
* Description:
*   This code demonstrates how to use UART library routines. Upon receiving
*   data via RS232, MCU immediately sends it back to the sender.
* Test configuration:
*   MCU:             ATxmega128A1
*                    http://www.atmel.com/dyn/resources/prod_documents/doc8067.pdf
*   Dev.Board:       MikroMedia for XMEGA
*                    http://www.mikroe.com/eng/products/view/688/mikrommb-for-xmega-board/
*   Oscillator:      Internal Clock, 32.0000 MHz
*   Modules:         USB UART on-board module
*                    ac:USB_UART
*   SW:              mikroC PRO for AVR
*                    http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/
* NOTES:
*    - None.
*******************************************************************************************/

sbit BLED_Direction            at PORTC_DIR.B4;
sbit BLED                      at PORTC_OUT.B4;
char uart_rd;

void main() {
  // ----- Internal Clock selection procedure!!!
  OSC_CTRL = 0x02;          // 32MHz internal RC oscillator

  while(RC32MRDY_bit == 0)
    ;

  CPU_CCP = 0xD8;
  CLK_CTRL = 1;             // set system clock to 32MHz internal RC oscillator

  // ---------------------------------

  BLED_Direction = 1;
  BLED = 0;

  UARTF0_Init(19200);              // Initialize UART module at 19200 bps
  Delay_ms(1000);                  // Wait for UART module to stabilize

  UARTF0_Write_Text("Start");

  while (1) {                      // Endless loop
  
   if (UARTF0_Data_Ready()) {      // If data is received,
     uart_rd = UARTF0_Read();      // read the received data,
     UARTF0_Write(uart_rd);        // and send data via UART
    }
  }
}