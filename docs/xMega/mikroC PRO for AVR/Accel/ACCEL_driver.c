/**************************************************************************************************
* File: ACCEL_driver.c
* File Type: C - Header File
* Project Name: BrmBrrrrm
* Company: (c) mikroElektronika, 2011
* Revision History:
*     20111020 (JK):
*       - initial release;
* Description:
*     This module contains a set of functions that are used for communication with
*     3-axis digital accelerometer ADXL345.
*
* Target:
* Test configuration:
*       MCU:             ATxmega128A1
*                        http://www.atmel.com/dyn/resources/prod_documents/doc8067.pdf
*       Dev.Board:       Mikromedia_for_XMEGA
*                        http://www.mikroe.com/eng/products/view/688/mikrommb-for-xmega-board/
*       Oscillator:      Internal Clock, 32.0000 MHz
*       SW:              mikroC PRO for AVR
*                        http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/
* NOTES:
*     TWID bus is used for communication with accel.
**************************************************************************************************/
#include "ACCEL_driver.h"

// ADXL345 Register Definition
#define _POWER_CTL      0x2D
#define _DATA_FORMAT    0x31
#define _BW_RATE        0x2C
#define _DATAX0         0x32
#define _DATAX1         0x33
#define _DATAY0         0x34
#define _DATAY1         0x35
#define _DATAZ0         0x36
#define _DATAZ1         0x37
#define _FIFO_CTL       0x38
#define _SPEED          0x0F          // Buffer Speed - 3200Hz

#define _ACCEL_ERROR    0x02

/**************************************************************************************************
* Function ADXL345_Write()
* -------------------------------------------------------------------------------------------------
* Overview: Function write byte of data to ADXL345
* Input: register address, data
* Output: Nothing
**************************************************************************************************/
void ADXL345_Write(unsigned short address, unsigned short data1)
{
  TWID_Start();              // issue I2C start signal
  TWID_Write(0x3A);             // send byte via I2C  (device address + W)
  TWID_Write(address);          // send byte (address of the location)
  TWID_Write(data1);            // send data (data to be written)
  TWID_Stop();               // issue I2C stop signal                     // issue I2C stop signal
}

/**************************************************************************************************
* Function ADXL345_Read()
* -------------------------------------------------------------------------------------------------
* Overview: Function read byte of data from ADXL345
* Input: register address
* Output: data from addressed register in ADXL345
**************************************************************************************************/
unsigned short ADXL345_Read(unsigned short address)
{
  unsigned short tmp = 0;

  TWID_Start();              // issue I2C start signal
  TWID_Write(0x3A);             // send byte via I2C (device address + W)
  TWID_Write(address);          // send byte (data address)

  TWID_Start();              // issue I2C signal repeated start
  TWID_Write(0x3B);             // send byte (device address + R)
  tmp = TWID_Read(0);          // Read the data (NO acknowledge)
  TWID_Stop();               // issue I2C stop signal

  return tmp;
}

/**************************************************************************************************
* Function ADXL345_Init()
* -------------------------------------------------------------------------------------------------
* Overview: Function that initializes ADXL345
* Input: Nothing
* Output: return 0 for OK; return _ACCEL_ERROR for ERROR
**************************************************************************************************/
char ADXL345_Init(void)
{
  char id = 0x00;

  // Go into standby mode to configure the device.
  ADXL345_Write(0x2D, 0x00);

  id = ADXL345_Read(0x00);
  if (id != 0xE5) {
    return _ACCEL_ERROR;
  }
  else {
    ADXL345_Write(_DATA_FORMAT, 0x08);       // Full resolution, +/-2g, 4mg/LSB, right justified
    ADXL345_Write(_BW_RATE, 0x0A);           // Set 100 Hz data rate
    ADXL345_Write(_FIFO_CTL, 0x80);          // stream mode
    ADXL345_Write(_POWER_CTL, 0x08);         // POWER_CTL reg: measurement mode
    return 0x00;
  }
}

/**************************************************************************************************
* Function Accel_ReadX()
* -------------------------------------------------------------------------------------------------
* Overview: Function read X axis from accel.
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
int Accel_ReadX(void)
{
  char low_byte;
  int Out_x;

  Out_x = ADXL345_Read(_DATAX1);
  low_byte = ADXL345_Read(_DATAX0);

  Out_x = (Out_x << 8);
  Out_x = (Out_x | low_byte);

  return Out_x;
}

/**************************************************************************************************
* Function Accel_ReadY()
* -------------------------------------------------------------------------------------------------
* Overview: Function read Y axis from accel.
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
int Accel_ReadY(void)
{
  char low_byte;
  int Out_y;

  Out_y = ADXL345_Read(_DATAY1);
  low_byte = ADXL345_Read(_DATAY0);

  Out_y = (Out_y << 8);
  Out_y = (Out_y | low_byte);

  return Out_y;
  
//  Hi(result) := ADXL345_Read(_DATAY1);
//  Lo(result) := ADXL345_Read(_DATAX0);
}

/**************************************************************************************************
* Function Accel_ReadZ()
* -------------------------------------------------------------------------------------------------
* Overview: Function read Z axis from accel.
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
int Accel_ReadZ(void)
{
  char low_byte;
  int Out_z;

  Out_z = ADXL345_Read(_DATAZ1);
  low_byte = ADXL345_Read(_DATAZ0);

  Out_z = (Out_z << 8);
  Out_z = (Out_z | low_byte);

  return Out_z;
}

/**************************************************************************************************
* End of File
**************************************************************************************************/