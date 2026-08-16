/**************************************************************************************************
* File: SF_driver.h
* File Type: C - Header File
* Project Name: BrmBrrrrm
* Company: (c) mikroElektronika, 2011
* Revision History:
*     20111118 (JK):
*       - initial release;
* Description:
*     This module contains a set of functions that are used for communication with
*     Serial Flash.
* Test configuration:
*       MCU:             ATxmega128A1
*                        http://www.atmel.com/dyn/resources/prod_documents/doc8067.pdf
*       Dev.Board:       Mikromedia_for_XMEGA
*                        http://www.mikroe.com/eng/products/view/688/mikromedia-for-xmega/
*       Oscillator:      Internal Clock, 32.0000 MHz
*       SW:              mikroC PRO for AVR
*                        http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/
* NOTES:
*     Mp3 codec use SPI bus to communicate with MCU.
**************************************************************************************************/

/**************************************************************************************************
* Serial Flash Chip Select connection
**************************************************************************************************/
extern sfr sbit CS_Serial_Flash_bit;
extern sfr sbit CS_Serial_Flash_Direction_bit;
/*************************************************************************************************/
// Constans
static const unsigned short _SERIAL_FLASH_CMD_RDID  = 0x9F;    // 25P80
static const unsigned short _SERIAL_FLASH_CMD_READ  = 0x03;
static const unsigned short _SERIAL_FLASH_CMD_WRITE = 0x02;
static const unsigned short _SERIAL_FLASH_CMD_WREN  = 0x06;
static const unsigned short _SERIAL_FLASH_CMD_RDSR  = 0x05;
static const unsigned short _SERIAL_FLASH_CMD_ERASE = 0xC7;    // 25P80
static const unsigned short _SERIAL_FLASH_CMD_EWSR  = 0x06;    // 25P80
static const unsigned short _SERIAL_FLASH_CMD_WRSR  = 0x01;
static const unsigned short _SERIAL_FLASH_CMD_SER   = 0xD8;    //25P80

// Functions
void SerialFlash_init();
void SerialFlash_WriteEnable();
unsigned char SerialFlash_IsWriteBusy();
void SerialFlash_WriteByte(unsigned char _data, unsigned long address);
void SerialFlash_WriteWord(unsigned int _data, unsigned long address);
unsigned char SerialFlash_ReadID(void);
unsigned char SerialFlash_ReadByte(unsigned long address);
unsigned int SerialFlash_ReadWord(unsigned long address);
unsigned char SerialFlash_WriteArray(unsigned long address, unsigned char* pData, unsigned int nCount);
void SerialFlash_ReadArray(unsigned long address, unsigned char* pData, unsigned int nCount);
void SerialFlash_ChipErase(void);
void SerialFlash_ResetWriteProtection();
void SerialFlash_SectorErase(unsigned long address);

/**************************************************************************************************
* End of File
**************************************************************************************************/