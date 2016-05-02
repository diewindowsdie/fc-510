//----------------------------------------------------------------------------

//LCD 1601/1602 support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Lcd.h"

//--------------------------- Константы: -------------------------------------

#ifdef LCD1602
//User symbols table:
//8 bytes per symbol, 8 symbols max
//Matrix 5x8: BYTE1 D4 D3 D2 D1 D0
//            BYTE2 D4 D3 D2 D1 D0
//            ...
//            BYTE8 D4 D3 D2 D1 D0
__flash char UsrChr[]=
{
  0x01,0x01,0x15,0x00,0x00,0x00,0x00,0x00, //0x01 - level 0
  0x01,0x01,0x15,0x00,0x10,0x10,0x10,0x10, //0x02 - level 1
  0x01,0x01,0x15,0x00,0x14,0x14,0x14,0x14, //0x03 - level 2
  0x01,0x01,0x15,0x00,0x15,0x15,0x15,0x15, //0x04 - level 3
  0x01,0x01,0x15,0x00,0x1E,0x1E,0x1E,0x1E, //0x05 - level 4
  0x01,0x01,0x15,0x00,0x1F,0x1F,0x1F,0x1F  //0x06 - level 5
};
#endif

//------------------------- Function prototypes: -----------------------------

void LCD_Wr4(char d);      //write nibble to LCD
void Delay_ms(int d);      //ms range delay
void LCD_Clear(void);      //LCD clear
#ifdef LCD1602
  void LCD_UsrChr(void);   //user symbols load
#endif

//------------------------ Write nibble to LCD: ------------------------------

void LCD_Wr4(char d)
{
  for(char i = 0; i < 5; i++)
  {
    Port_SCLK_0;
    if(d & 0x10) Port_DATA_1;
      else Port_DATA_0;
    d = d << 1;
    Port_SCLK_1;
  }
  Port_LOAD_1;         //E <- 1
  Port_DATA_1;
  Delay_us(2);         //delay 2 us
  Port_LOAD_0;         //E <- 0
}

//-------------------------- ms range delay: ---------------------------------

void Delay_ms(int d)
{
  while(d)
  {
    Delay_us(1000);
    __watchdog_reset();
    d--;
  }
}

//------------------------------ LCD clear: ----------------------------------

void LCD_Clear(void)
{
  LCD_WrCmd(0x01);     //DISPLAY CLEAR
  Delay_ms(5);         //delay >1.64mS
}

//---------------------------- Load LCD CGRAM: -------------------------------

#ifdef LCD1602
void LCD_UsrChr(void)
{
  LCD_WrCmd(0x40 + 8);     //set CGRAM address = 8
  for(char i = 0; i < 6 * 8; i++)
    LCD_WrData(UsrChr[i]); //load CGRAM
}
#endif

//----------------------------------------------------------------------------
//--------------------------- Exported functions: ----------------------------
//----------------------------------------------------------------------------

//-------------------------------- LCD init: ---------------------------------

void LCD_Init(void)
{
  Port_LOAD_0;         //E <- 0
  Delay_ms(15);
  LCD_WrCmd(0x30);
  Delay_ms(5);         //delay >4.1 ms
  LCD_WrCmd(0x30);
  Delay_us(100);       //delay >100 us
  LCD_WrCmd(0x30);
  Delay_ms(5);         //delay >4.1 ms
  LCD_WrCmd(0x20);     //FUNCTION SET (8 bit)
  Delay_ms(15);
  LCD_WrCmd(0x28);     //FUNCTION SET (4 bit)
  Delay_ms(15);
  LCD_WrCmd(0x06);     //ENTRY MODE SET
  Delay_ms(15);
  LCD_Clear();         //CLEAR
  Delay_ms(15);
  LCD_WrCmd(0x0C);     //DISPLAY ON
  Delay_ms(15);
#ifdef LCD1602
  LCD_UsrChr();        //user symbols load
#endif
}

//--------------------------- Set LCD position: ------------------------------

//pos = 1..16

void LCD_Pos(char pos)
{
  pos = pos - 1;
#ifdef LCD1601
  if(pos > 7)
    pos = (pos & 0x07) | 0x40;
#endif
  pos = pos | 0x80;
  LCD_WrCmd(pos);
}

//--------------------- Write command to LCD (RS = 0): -----------------------

void LCD_WrCmd(char d)
{
  LCD_Wr4(__swap_nibbles(d) & 0x0F);
  Delay_us(10);
  LCD_Wr4(d & 0x0F);
  Delay_us(50);
}

//---------------------- Write data to LCD (RS = 1): -------------------------

void LCD_WrData(char d)
{
  LCD_Wr4(__swap_nibbles(d) | 0x10);
  Delay_us(10);
  LCD_Wr4(d | 0x10);
  Delay_us(50);
}

//----------------------------------------------------------------------------
