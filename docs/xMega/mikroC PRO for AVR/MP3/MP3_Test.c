/**************************************************************************************************
* File: MP3_Test.c
* File Type: C - Source Code File
* (C) mikroElektronika, 2010-2011
* Revision History:
*     20111117:
*       - modified for mikroMedia for XMEGA HW revision 1.11 (JK);
* Description:
*     This project demonstrates communication with VS1053B mp3 codec.
*     Program reads one mp3 file from MMC and sends it to VS1053B for decoding
*     and playing.
*     MMC and MP3_SCI share Hardware SPI module.
* Test configuration:
*       MCU:             ATxmega128A1
*                        http://www.atmel.com/dyn/resources/prod_documents/doc8067.pdf
*       Dev.Board:       Mikromedia_for_XMEGA
*                        http://www.mikroe.com/eng/products/view/688/mikromedia-for-xmega/
*       Modules:         MP3 VS1053B on-board module
*                        ac:MP3
*       Oscillator:      Internal Clock, 32.0000 MHz
*       SW:              mikroC PRO for AVR
*                        http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/
* NOTES:
*     - Make sure that MMC card is properly formatted (to FAT16 or just FAT)
*       before testing it on this example.
*     - Make sure that MMC card contains appropriate mp3 file ( sound.mp3 ).
**************************************************************************************************/
#include <built_in.h>
#include "MP3_Test.h"
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
sbit MP3_CS                    at PORTH_OUT.B6;
sbit MP3_RST                   at PORTH_OUT.B5;
sbit BSYNC                     at PORTH_OUT.B4;
sbit DREQ                      at PORTE_IN.B5;

sbit MP3_CS_Direction          at PORTH_DIR.B6;
sbit MP3_RST_Direction         at PORTH_DIR.B5;
sbit BSYNC_Direction           at PORTH_DIR.B4;
sbit DREQ_Direction            at PORTE_DIR.B5;

/**************************************************************************************************
* Function MCU_Init()
* -------------------------------------------------------------------------------------------------
* Overview: Function Initialize MCU
* Input: Nothing
* Output: Nothing
**************************************************************************************************/

void InitMCU(){
  OSC_CTRL = 0x02;          // 32MHz internal RC oscillator

  while(RC32MRDY_bit == 0)
    ;

  CPU_CCP = 0xD8;
  CLK_CTRL = 1;             // set system clock to 32MHz internal RC oscillator
  Delay_500us();

  TFT_Set_Default_Mode();
  TFT_Init(320, 240);
  Delay_ms(100);
  
  TFT_BLED_Direction = 1;
}
/*********************************************************************************************/
void DrawMP3Scr(){
  TFT_Fill_Screen(CL_WHITE);
  TFT_Set_Pen(CL_Black, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_LIne(20,  46, 300,  46);
  TFT_Set_Font(&HandelGothic_BT21x22_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("MP3  TEST", 75, 14);
  TFT_Set_Font(&Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("mikromedia for XMEGA", 19, 223);
  TFT_Set_Font(&Verdana12x13_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);
  TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
}
/**************************************************************************************************
* Function MC3_Init()
* -------------------------------------------------------------------------------------------------
* Overview: Function Initialize MP3 VS1053B codec
* Input: Nothing
* Output: Nothing
**************************************************************************************************/

void MP3_Init(void)
{
  BSYNC = 1;
  MP3_CS = 1;

  // Hardware reset
  MP3_RST = 0;
  Delay_ms(10);
  MP3_RST = 1;
  
  while (DREQ == 0);

  MP3_SCI_Write(SCI_MODE_ADDR, 0x0800);
  MP3_SCI_Write(SCI_BASS_ADDR, 0x7A00);
  MP3_SCI_Write(SCI_CLOCKF_ADDR, 0x2000);   // default 12 288 000 Hz

  // Maximum volume is 0x00 and total silence is 0xFE.
  volume_left  = 0; //0x3F;
  volume_right = 0; //0x3F;
  MP3_Set_Volume(volume_left, volume_right);
}

/**************************************************************************************************
* Function MP3_Start()
* -------------------------------------------------------------------------------------------------
* Overview: Function Initialize SPI to communicate with MP3 codec
* Input: Nothing
* Output: Nothing
**************************************************************************************************/
void MP3_Start(void)
{
  // Disable other peripheral modules on the same SPI bus
  CS_Serial_Flash_Direction_bit = 1;
  CS_Serial_Flash_bit = 1;             // Disable Serial Flash module

  MP3_CS_Direction  = 1;               // Configure MP3_CS as output
  MP3_CS            = 1;               // Deselect MP3_CS
  MP3_RST_Direction = 1;               // Configure MP3_RST as output
  MP3_RST           = 1;               // Set MP3_RST pin

  DREQ_Direction    = 0;               // Configure DREQ as input
  BSYNC_Direction   = 1;               // Configure BSYNC as output
  BSYNC             = 0;               // Clear BSYNC
  BSYNC             = 1;               // Clear BSYNC

  // Initialize SPIC module
  // master_mode                   = _SPI_MASTER
  // Clock rate                    = _SPI_FCY_DIV16
  // SPI clock polarity and phase  = _SPI_CLK_LO_LEADING
  SPIC_Init_Advanced(_SPI_MASTER, _SPI_FCY_DIV16, _SPI_CLK_LO_LEADING);
  TFT_BLED = 1;
  
  TFT_Set_Pen(CL_WHITE, 1);
  TFT_Set_Brush(1, CL_WHITE, 0, 0, 0, 0);

  TFT_Write_Text("1. Initializing VS1053B decoder interface", 30, 80);

  MP3_Init();
  Delay_ms(1000);
}

/**************************************************************************************************
* Function MP3_Test()
* -------------------------------------------------------------------------------------------------
* Overview: Function run the MP3 test
* Input: Nothing
* Output: test status: 0 - skiped; 1 - pass; 2 - fail
**************************************************************************************************/
void MP3_Test(char *test)
{
  unsigned long i;

  // Reset error flag
  *test = 0;

  TFT_Write_Text("2. Initializing MMC_FAT", 30, 100);
  if (Mmc_Fat_Init() == 0) {
    if (Mmc_Fat_Assign(&mp3_filename, 0) ) {
      TFT_Write_Text("3. File Assigned", 30, 120);
      Mmc_Fat_Reset(&file_size);          // Call Reset before file reading,
                                          //   procedure returns size of the file

      TFT_Write_Text("4. Play audio... :)", 30, 140);
      // send file blocks to MP3 SDI
      while (file_size > BUFFER_SIZE)
      {
        for (i=0; i<BUFFER_SIZE; i++)
        {
          Mmc_Fat_Read(mp3_buffer + i);
        }
        for (i=0; i<BUFFER_SIZE/BYTES_2_WRITE; i++) {
          MP3_SDI_Write_32(mp3_buffer + i*BYTES_2_WRITE);
        }

        file_size -= BUFFER_SIZE;

//        Check_TP();
//        if (ucMP3_run_test == 0)return;
     }

      // send the rest of the file to MP3 SDI
      for (i=0; i<file_size; i++)
      {
        Mmc_Fat_Read(mp3_buffer + i);
      }

      for (i=0; i<file_size; i++)
      {
        MP3_SDI_Write(mp3_buffer[i]);
      }

      TFT_Write_Text("5. Finish!", 30, 160);
    }
    else {

      TFT_Write_Text("3. File not assigned", 30, 120);
      *test = 2;
    }
  }
  else {
    TFT_Write_Text("3. MMC FAT not initialized", 30, 120);
    *test = 2;
  }
}

/**************************************************************************************************
* main function
**************************************************************************************************/

void main(){
  InitMCU();
  DrawMP3Scr();
  
  MP3_Start();
  MP3_Test(&ucMP3_run_test);
}