/**************************************************************************************************
* File: ACCEL_Test.h
* File Type: C - Header File
* Project Name: BrmBrrrrm
* Company: (c) mikroElektronika, 2011
* Revision History:
*     20111118 (JK):
*       - initial release;
* Description:
*     This module demonstrates communication with 3-axis digital
*     accelerometer ADXL345 on mmB PIC32. Values from accelerometer
*     are shown on the TFT display.
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

// Global Variables
char out[16];
int readings[3] = {0, 0, 0}; // X, Y and Z buffer

// Calculate the average values of the X, Y and Z axis reads
void Accel_Average(void);
// Display average X-axis read value on TFT
void Display_X_Value(void);
// Display average Y-axis read value on TFT
void Display_Y_Value(void);
// Display average z-axis read value on TFT
void Display_Z_Value(void);
// Start using ACCEL
void ACCEL_Start(char *test);
// Test Accel
void ACCEL_Test(void);

//- Imported from ACCEL_DRIVER.H ------------------------------------------------------------------
extern void ADXL345_Write(unsigned short address, unsigned short data1);
extern unsigned short ADXL345_Read(unsigned short address);
extern char ADXL345_Init(void);

extern int Accel_ReadX(void);
extern int Accel_ReadY(void);
extern int Accel_ReadZ(void);

/**************************************************************************************************
* End of File
**************************************************************************************************/