/**************************************************************************************************
* File: MP3_driver.c
* File Type: C - Source Code File
* Company: (c) mikroElektronika, 2011-2011
* Revision History:
*     20111118:
*       - modified for mikroMedia PIC32 for XMEGA revision 1.11 (JK);
* Description:
*     This module contains a set of functions that are used for communication with
*     VS1053E mp3 codec.
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
#include <built_in.h>
#include "MP3_driver.h"

/**************************************************************************************************
* CODEC V1053E connections
**************************************************************************************************/
extern sfr sbit Mmc_Chip_Select;
extern sfr sbit MP3_CS;
extern sfr sbit MP3_RST;
extern sfr sbit BSYNC;
extern sfr sbit DREQ;

extern sfr sbit Mmc_Chip_Select_Direction;
extern sfr sbit MP3_CS_Direction;
extern sfr sbit MP3_RST_Direction;
extern sfr sbit BSYNC_Direction;
extern sfr sbit DREQ_Direction;

/**************************************************************************************************
* Function MP3_SCI_Write()
* -------------------------------------------------------------------------------------------------
* Overview: Function writes one byte to MP3 SCI
* Input: register address in codec, data
* Output: Nothing
**************************************************************************************************/
void MP3_SCI_Write(char address, unsigned int data_in) {
  BSYNC = 1;

  MP3_CS = 0;                    // select MP3 SCI
  SPIC_Write(WRITE_CODE);
  SPIC_Write(address);
  SPIC_Write(Hi(data_in));       // high byte
  SPIC_Write(Lo(data_in));       // low byte
  MP3_CS = 1;                    // deselect MP3 SCI
  while (DREQ == 0);             // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI
}

/**************************************************************************************************
* Function MP3_SCI_Read()
* -------------------------------------------------------------------------------------------------
* Overview: Function reads words_count words from MP3 SCI
* Input: start address, word count to be read
* Output: words are stored to data_buffer
**************************************************************************************************/
void MP3_SCI_Read(char start_address, char words_count, unsigned int *data_buffer) {
  unsigned int temp;

  MP3_CS = 0;                    // select MP3 SCI
  SPIC_Write(READ_CODE);
  SPIC_Write(start_address);

  while (words_count--) {        // read words_count words byte per byte
    temp = SPIC_Read(0);
    temp <<= 8;
    temp += SPIC_Read(0);
    *(data_buffer++) = temp;
  }
  MP3_CS = 1;                    // deselect MP3 SCI
  while (DREQ == 0);             // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI
}

/**************************************************************************************************
* Function MP3_SDI_Write()
* -------------------------------------------------------------------------------------------------
* Overview: Function write one byte to MP3 SDI
* Input: data to be writed
* Output: Nothing
**************************************************************************************************/
void MP3_SDI_Write(char data_) {

  MP3_CS = 1;
  BSYNC = 0;

  while (DREQ == 0);             // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI

  SPIC_Write(data_);
  BSYNC = 1;
}

/**************************************************************************************************
* Function MP3_SDI_Write_32
* -------------------------------------------------------------------------------------------------
* Overview: Function Write 32 bytes to MP3 SDI
* Input: data buffer
* Output: Nothing
**************************************************************************************************/
void MP3_SDI_Write_32(char *data_) {
  char i;

  MP3_CS = 1;
  BSYNC = 0;

  while (DREQ == 0);             // wait until DREQ becomes 1, see MP3 codec datasheet, Serial Protocol for SCI

  for (i=0; i<32; i++)
  SPIC_Write(data_[i]);
  BSYNC = 1;
}

/**************************************************************************************************
* Function MP3_Set_Volume()
* -------------------------------------------------------------------------------------------------
* Overview: Function set volume on the left and right channel
* Input: left channel volume, right channel volume
* Output: Nothing
**************************************************************************************************/
void MP3_Set_Volume(char left, char right) {
  unsigned int volume;

  volume = (left<<8) + right;             // calculate value
  MP3_SCI_Write(SCI_VOL_ADDR, volume);    // Write value to VOL register
}

/**************************************************************************************************
* End of File
**************************************************************************************************/