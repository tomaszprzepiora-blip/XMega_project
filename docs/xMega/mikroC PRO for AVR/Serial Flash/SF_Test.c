/**************************************************************************************************
* File: SF_Test.c
* File Type: C - Source Code File
* Company: (c) mikroElektronika, 2011
* Revision History:
*     20111115 (DA):
*       - initial release;
*     20111117 (JK);
*       - revision;
* Description:
*     This module contains a set of functions that are used for communication with
*     Serial Flash.
* Test configuration:
*       MCU:             ATxmega128A1
*                        http://www.atmel.com/dyn/resources/prod_documents/doc8067.pdf
*       Dev.Board:       Mikromedia_for_XMEGA
*                        http://www.mikroe.com/eng/products/view/688/mikromedia-for-xmega/
*       Modules:         Serial Flash M25P80 on-board module
*                        ac:Serial_Flash
*       Oscillator:      Internal Clock, 32.0000 MHz
*       SW:              mikroC PRO for AVR
*                        http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/
* NOTES:
*     Serial Flash use SPI bus to communicate with MCU.
**************************************************************************************************/
#define _DATA_ARRAY_SIZE 16
#include <built_in.h>
#include "SF_Test.h"
#include "resources.h"
/**************************************************************************************************
* MMC Chip Select connection
**************************************************************************************************/
sbit Mmc_Chip_Select_Direction at PORTF_DIR.B5;
sbit Mmc_Chip_Select           at PORTF_OUT.B5;

/**************************************************************************************************
* Serial Flash Chip Select connection
**************************************************************************************************/
sbit CS_Serial_Flash_bit           at PORTB_OUT.B7;
sbit CS_Serial_Flash_Direction_bit at PORTB_DIR.B7;

/**************************************************************************************************
* CODEC V1053E connections
**************************************************************************************************/
sbit MP3_CS_Direction          at PORTH_DIR.B6;
sbit MP3_CS                    at PORTH_OUT.B6;
/*************************************************************************************************/
char cSF_test_status;
/*************************************************************************************************
* Init MCU function
**************************************************************************************************/
void InitMCU(){
  OSC_CTRL = 0x02;          // 32MHz internal RC oscillator

  while(RC32MRDY_bit == 0)
    ;

  CPU_CCP = 0xD8;
  CLK_CTRL = 1;             // set system clock to 32MHz internal RC oscillator
  Delay_500us();

  TFT_Set_Default_Mode();
  TFT_Init_ILI9341_8bit(320, 240);
  Delay_ms(1000);

  TFT_BLED_Direction = 1;
}
/*********************************************************************************************/
void DrawSFScr(){
  TFT_Fill_Screen(CL_WHITE);
  TFT_Set_Pen(CL_Black, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_LIne(20,  46, 300,  46);
  TFT_Set_Font(&HandelGothic_BT21x22_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("SERIAL  FLASH", 75, 14);
  TFT_Set_Font(&Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("mikromedia for XMEGA", 19, 223);
  TFT_Set_Font(&Verdana12x13_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);
  TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
}
/**************************************************************************************************
* Function SF_Start()
* -------------------------------------------------------------------------------------------------
* Overview: Function Initialize SPI bus and serial flash
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void SF_Start(void)
{
  // Disable other peripheral modules on the same SPI bus
  Mmc_Chip_Select_Direction = 1;
  Mmc_Chip_Select = 1;            // Disable MMC module

  MP3_CS_Direction = 1;
  MP3_CS = 1;                     // Disable MP3 module

  // Initialize SPIC module
  // master_mode                   = _SPI_MASTER
  // Clock rate                    = _SPI_FCY_DIV4
  // SPI clock polarity and phase  = _SPI_CLK_LO_LEADING
  SPIC_Init_Advanced(_SPI_MASTER, _SPI_FCY_DIV4, _SPI_CLK_LO_LEADING);
  TFT_BLED = 1;

  TFT_Set_Font(TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
  TFT_Set_Pen(CL_WHITE, 1);
  TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);

  SerialFlash_init();
  SerialFlash_WriteEnable();
  Delay_ms(500);
}

/**************************************************************************************************
* Function SF_Test()
* -------------------------------------------------------------------------------------------------
* Overview: Function run the serial flash test
* Input: Nothing
* Output: test status: 0 - skiped; 1 - pass; 2 - fail
**************************************************************************************************/
void SF_Test(char *test)
{
  unsigned char temp, SerialFlashID, txt[12];
  unsigned char i, success;

  // Reset error flag
  *test = 0;

  // Erase entire flash
  TFT_Write_Text("1. Erasing chip... ", 90, 65);
  SerialFlash_ChipErase();
  TFT_Write_Text("Done!", 200, 65);
 
  // Read Flash ID
  SerialFlashID = SerialFlash_ReadID();
  ByteToStr(SerialFlashID, txt);
  TFT_Write_Text("2. Flash ID:", 90, 85);
  TFT_Write_Text(txt, 170, 85);
  Delay_ms(500);

  // Write and read back a single byte
  temp = 221;
  TFT_Write_Text("3. Writting:", 90, 105);
  ByteToStr(temp, txt);
  TFT_Write_Text(txt, 170, 105);
  SerialFlash_WriteByte(temp, 0x123456);

  TFT_Write_Text("4. Reading: ", 90, 125);
  temp = SerialFlash_ReadByte(0x123456);
  ByteToStr(temp, txt);
  TFT_Write_Text(txt, 170, 125);
  Delay_ms(500);

  // Write the repetitive pattern of data to fill the first 4K of memory
  // and then read the entire flash again and check the data correctness
  TFT_Write_Text("5. Writing array...", 90, 145);
  SerialFlash_WriteArray(0x0000, &write_array, _DATA_ARRAY_SIZE);
  Delay_ms(500);
  
  TFT_Write_Text("6. Reading array...", 90, 165);
  SerialFlash_ReadArray(0x0000, &read_array, _DATA_ARRAY_SIZE);
  Delay_ms(500);
  success = 1;
  
  for (i = 0; i<_DATA_ARRAY_SIZE; i++)
  {
    if (read_array[i] != write_array[i])
    {
      success = 0;
      break;
    }
  }
  
  if (success)
  {
    TFT_Write_Text("7. Success - Full match!", 90, 185);
    *test = 1;
  }
  else
  {
    TFT_Write_Text("7. Failed", 90, 185);
    *test = 2;
  }
}

/**************************************************************************************************
* main function
**************************************************************************************************/

void main(){
  InitMCU();
  DrawSFScr();
  SF_Start();
  SF_Test(&cSF_test_status);
}