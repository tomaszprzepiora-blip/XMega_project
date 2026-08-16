/**************************************************************************************************
* File: MMC_Test.c
* File Type: C - Source Code File
* Company: (c) mikroElektronika, 2010 - 2011
* Revision History:
*     20111020:
*       - modified for mikroMedia for XMEGA HW revision 1.11 (JK);
*
* Description:
*     This module consists of several blocks that demonstrate various aspects of
*     usage of the Mmc library. These are:
*     - Creation of new file and writing down to it;
*     - Opening existing file and re-writing it (writing from start-of-file);
*     - Opening existing file and appending data to it (writing from end-of-file);
*     - Opening a file and reading data from it (sending it to USART terminal);
*     - Creating and modifying several files at once;
*     - Reading file contents;
*     - Deleting file(s);
*     - Creating the swap file (see Help for details);
*
* Test configuration:
*       MCU:             ATxmega128A1
*                        http://www.atmel.com/dyn/resources/prod_documents/doc8067.pdf
*       Dev.Board:       Mikromedia_for_XMEGA
*                        http://www.mikroe.com/eng/products/view/688/mikromedia-for-xmega/
*       Modules:         on-board microSD card holder
*                        ac:MMC_SD
*       Oscillator:      Internal Clock, 32.0000 MHz
*       SW:              mikroC PRO for AVR
*                        http://www.mikroe.com/eng/products/view/228/mikroc-pro-for-avr/
*
* NOTES:
*     - On TFT are displayed only messages. Context of created files are sent to UART1 module
*       To view UART messages use USB UART module
*     - Make sure that MMC card is properly formatted (to FAT16 or just FAT)
*       before testing it on this example.
*     - This example expects MMC card to be inserted before reset, otherwise,
*       the FAT_ERROR message is displayed.
**************************************************************************************************/
#include "resources.h"
/**************************************************************************************************
* MMC Chip Select connection
**************************************************************************************************/
sbit Mmc_Chip_Select           at PORTF_OUT.B5;
sbit Mmc_Chip_Select_Direction at PORTF_DIR.B5;

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
  TFT_BLED = 1;
}
/*************************************************************************************************/
void DrawMMCScr(){
  TFT_Fill_Screen(CL_WHITE);
  TFT_Set_Pen(CL_Black, 1);
  TFT_Line(20, 220, 300, 220);
  TFT_LIne(20,  46, 300,  46);
  TFT_Set_Font(&HandelGothic_BT21x22_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("MMC  TEST", 75, 14);
  TFT_Set_Font(&Verdana12x13_Regular, CL_BLACK, FO_HORIZONTAL);
  TFT_Write_Text("mikromedia for XMEGA", 19, 223);
  TFT_Set_Font(&Verdana12x13_Regular, CL_RED, FO_HORIZONTAL);
  TFT_Write_Text("www.mikroe.com", 200, 223);
  TFT_Set_Font(&TFT_defaultFont, CL_BLACK, FO_HORIZONTAL);
}
/*********************************************************************************************/

const LINE_LEN = 43;
char err_txt[20]       = "FAT16 not found";
char file_contents[LINE_LEN] = "XX MMC/SD FAT16 library by Anton Rieckert\n";
char           filename[14] = "MIKRO00x.TXT";          // File names
unsigned short loop, loop2;
unsigned long  i, size;
char           Buffer[512];

// UARTF write text and new line (carriage return + line feed)
void UARTF0_Write_Line(char *uart_text) {
  UARTF0_Write_Text(uart_text);
  UARTF0_Write(13);
  UARTF0_Write(10);
}

// Creates new file and writes some data to it
void M_Create_New_File() {
  filename[7] = 'A';
  Mmc_Fat_Set_File_Date(2010, 4, 19, 9, 0, 0); // Set file date & time info
  Mmc_Fat_Assign(&filename, 0xA0);          // Find existing file or create a new one
  Mmc_Fat_Rewrite();                        // To clear file and start with new data
  for(loop = 1; loop <= 99; loop++) {
    UARTF0_Write('.');
    file_contents[0] = loop / 10 + 48;
    file_contents[1] = loop % 10 + 48;
    Mmc_Fat_Write(file_contents, LINE_LEN-1);   // write data to the assigned file
  }
}

// Creates many new files and writes data to them
void M_Create_Multiple_Files() {
  for(loop2 = 'B'; loop2 <= 'Z'; loop2++) {
    UARTF0_Write(loop2);                  // signal the progress
    filename[7] = loop2;                 // set filename
    Mmc_Fat_Set_File_Date(2010, 4, 19, 9, 0, 0); // Set file date & time info
    Mmc_Fat_Assign(&filename, 0xA0);     // find existing file or create a new one
    Mmc_Fat_Rewrite();                   // To clear file and start with new data
    for(loop = 1; loop <= 44; loop++) {
      file_contents[0] = loop / 10 + 48;
      file_contents[1] = loop % 10 + 48;
      Mmc_Fat_Write(file_contents, LINE_LEN-1);  // write data to the assigned file
    }
  }
}

// Opens an existing file and rewrites it
void M_Open_File_Rewrite() {
  filename[7] = 'C';
  Mmc_Fat_Assign(&filename, 0);
  Mmc_Fat_Rewrite();
  for(loop = 1; loop <= 55; loop++) {
    file_contents[0] = loop / 10 + 48;
    file_contents[1] = loop % 10 + 48;
    Mmc_Fat_Write(file_contents, LINE_LEN-1);    // write data to the assigned file
  }
}

// Opens an existing file and appends data to it
//               (and alters the date/time stamp)
void M_Open_File_Append() {
   filename[7] = 'B';
   Mmc_Fat_Assign(&filename, 0);
   Mmc_Fat_Set_File_Date(2010, 4, 19, 9, 20, 0);
   Mmc_Fat_Append();                                    // Prepare file for append
   Mmc_Fat_Write(" for mikroElektronika 2011\n", 27);   // Write data to assigned file
}

// Opens an existing file, reads data from it and puts it to UART
void M_Open_File_Read() {
  char character;

  filename[7] = 'B';
  Mmc_Fat_Assign(&filename, 0);
  Mmc_Fat_Reset(&size);            // To read file, procedure returns size of file
  for (i = 1; i <= size; i++) {
    Mmc_Fat_Read(&character);
    UARTF0_Write(character);        // Write data to UART
  }
}

// Deletes a file. If file doesn't exist, it will first be created
// and then deleted.
void M_Delete_File() {
  filename[7] = 'F';
  Mmc_Fat_Assign(filename, 0);
  Mmc_Fat_Delete();
}

// Tests whether file exists, and if so sends its creation date
// and file size via UART
void M_Test_File_Exist() {
  unsigned long  fsize;
  unsigned int   year;
  unsigned short month, day, hour, minute;
  unsigned char  outstr[12];

  filename[7] = 'B';
//  filename[7] = 'F';
  if (Mmc_Fat_Assign(filename, 0)) {
    //--- file has been found - get its create date
    Mmc_Fat_Get_File_Date(&year, &month, &day, &hour, &minute);
    UARTF0_Write_Text(" created: ");
    WordToStr(year, outstr);
    UARTF0_Write_Text(outstr);
    ByteToStr(month, outstr);
    UARTF0_Write_Text(outstr);
    WordToStr(day, outstr);
    UARTF0_Write_Text(outstr);
    WordToStr(hour, outstr);
    UARTF0_Write_Text(outstr);
    WordToStr(minute, outstr);
    UARTF0_Write_Text(outstr);

    //--- file has been found - get its modified date
    Mmc_Fat_Get_File_Date_Modified(&year, &month, &day, &hour, &minute);
    UARTF0_Write_Text(" modified: ");
    WordToStr(year, outstr);
    UARTF0_Write_Text(outstr);
    ByteToStr(month, outstr);
    UARTF0_Write_Text(outstr);
    WordToStr(day, outstr);
    UARTF0_Write_Text(outstr);
    WordToStr(hour, outstr);
    UARTF0_Write_Text(outstr);
    WordToStr(minute, outstr);
    UARTF0_Write_Text(outstr);

    //--- get file size
    fsize = Mmc_Fat_Get_File_Size();
    LongToStr((signed long)fsize, outstr);
    UARTF0_Write_Line(outstr);
  }
  else {
    //--- file was not found - signal it
    UARTF0_Write(0x55);
    Delay_ms(1000);
    UARTF0_Write(0x55);
  }
}
// Tries to create a swap file, whose size will be at least 100
// sectors (see Help for details)
void M_Create_Swap_File() {
  unsigned int i;

  for(i=0; i<512; i++)
    Buffer[i] = i;

  size = Mmc_Fat_Get_Swap_File(5000, "mikroE.txt", 0x20);   // see help on this function for details

  if (size) {
    LongToStr((signed long)size, err_txt);
    UARTF0_Write_Line(err_txt);

    for(i=0; i<5000; i++) {
      Mmc_Write_Sector(size++, Buffer);
      UARTF0_Write('.');
    }
  }
}

// Main. Uncomment the function(s) to test the desired operation(s)
void main() {
  #define COMPLETE_EXAMPLE         // comment this line to make simpler/smaller example
  InitMCU();
  DrawMMCScr();

  //Disable other peripheral modules on the same SPI bus
  CS_Serial_Flash_Direction_bit = 0;
  CS_Serial_Flash_bit = 1;        // Disable Serial Flash module
  MP3_CS_Direction = 0;
  MP3_CS = 1;                     // Disable MP3 module
  
  // Initialize UARTF0 module
  UARTF0_Init(19200);
  Delay_ms(10);

  UARTF0_Write_Line("XMEGA-Started"); // PIC present report
  TFT_Write_Text("1. Check for MMC...", 90, 75);

  // Initialize SPI1 module
  SPIC_Init_Advanced(_SPI_MASTER, _SPI_FCY_DIV128, _SPI_CLK_LO_LEADING);
  TFT_BLED = 1;                           // Backlight ON

  // use fat16 quick format instead of init routine if a formatting is needed
  if (Mmc_Fat_Init() == 0) {
    TFT_Write_Text("2. MMC is ready to use!", 90, 105);
    // reinitialize spi at higher speed
    SPIC_Init_Advanced(_SPI_MASTER, _SPI_FCY_DIV4, _SPI_CLK_LO_LEADING);
    TFT_BLED = 1;                         // Backlight ON
    //--- Test start
    TFT_Write_Text("3. Test Start", 90, 135);
    UARTF0_Write_Line("Test Start.");
    //--- Test routines. Uncomment them one-by-one to test certain features
    M_Create_New_File();
    #ifdef COMPLETE_EXAMPLE
      M_Create_Multiple_Files();
      M_Open_File_Rewrite();
      M_Open_File_Append();
      M_Open_File_Read();
      M_Delete_File();
      M_Test_File_Exist();
      M_Create_Swap_File();
    #endif
    UARTF0_Write_Line("Test End.");
    TFT_Write_Text("4. Done!", 90, 165);
  }
  else
  {
    TFT_Write_Text("2. FAT16 not found!", 90, 105);
    UARTF0_Write_Line(err_txt); // Note: Mmc_Fat_Init tries to initialize a card more than once.
                               //       If card is not present, initialization may last longer (depending on clock speed)
  }

}