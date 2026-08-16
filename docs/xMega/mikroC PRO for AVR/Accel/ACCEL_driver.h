/**************************************************************************************************
* File: ACCEL_driver.h
* File Type: C - Header File
* Project Name: BrmBrrrrm
* Company: (c) mikroElektronika, 2011
* Revision History:
*     20111118 (JK):
*       - initial release;
* Description:
*     This module contains a set of functions that are used for communication with
*     3-axis digital accelerometer ADXL345.
*
* Test configuration:
*       MCU:             ATxmega128A1
*                        http://www.atmel.com/dyn/resources/prod_documents/doc8067.pdf
*       Dev.Board:       Mikromedia_for_XMEGA
*                        http://www.mikroe.com/eng/products/view/688/mikromedia-for-xmega/
*       Oscillator:      Internal Clock, 32.0000 MHz
*       SW:              mikroC PRO for AVR
*                        http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/
* NOTES:
*     TWID bus is used for communication with accel.
**************************************************************************************************/

// Functions
void ADXL345_Write(unsigned short address, unsigned short data1);
unsigned short ADXL345_Read(unsigned short address);
char ADXL345_Init(void);

// Read X Axis
int Accel_ReadX(void);

// Read Y Axis
int Accel_ReadY(void);

// Read Z Axis
int Accel_ReadZ(void);

/**************************************************************************************************
* End of File
**************************************************************************************************/