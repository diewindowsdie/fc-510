//----------------------------------------------------------------------------

//Keyboard support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Keyboard.h"

//----------------------------- Constants: -----------------------------------

#define DBOUNCE_TIME            50 //debounce time, ms
#define AUTOREPEAT_DELAY       800 //initial autorepeat delay, ms
#define AUTOREPEAT_SLOW_RATE   180 //slow autorepeat rate, ms
#define AUTOREPEAT_SLOW_COUNT   16 //fast autorepeat step count
#define AUTOREPEAT_FAST_RATE    60 //fast autorepeat rate, ms

//------------------------------ Variables: ----------------------------------

static char KeyCode;

//---------------------------- Keyboard init: --------------------------------

void Keyboard_Init(void)
{
  KeyCode = KEY_NO;
}

//---------------------------- Scan keyboard: --------------------------------

char Keyboard_Scan(void)
{
  char d = 0xEF;
  for(char i = 0; i < 8; i++)
  {
    Port_SCLK_0;
    if(d & 0x80) Port_DATA_1;
      else Port_DATA_0;
    d = d << 1;
    Port_SCLK_1;
    if(Pin_RETL) d = d | 1;
  }
  return(~d & 0x0F);
}

//-------------------------- Keyboard processing: ----------------------------

void Keyboard_Exe(bool t)
{
  static char LastCode = KEY_NO;
  static char TempCode = KEY_NO;
  static char DbncTimer = 0;
  static int  RepTimer = 0;
  static char RepCnt = 0;

  if(t)
  {
    if(DbncTimer) DbncTimer--;
    if(RepTimer) RepTimer--;
    char k = Keyboard_Scan();
    if(k != LastCode) //new press
    {
      if(k != TempCode) //bounce
      {
        TempCode = k;
        DbncTimer = ms2sys(DBOUNCE_TIME);
      }
      else
      {
        if(!DbncTimer)
        {
          if(k != KEY_NO) //key pressed
          {
            RepTimer = ms2sys(AUTOREPEAT_DELAY);
            KeyCode = k;
          }
          LastCode = k;
          RepCnt = 0;
        }
      }
    }
    else //key holded
    {
      if(RepTimer == 1) //repeat timer is over
      {
        KeyCode = k | REP_R; //autorepeat
        if(RepCnt < AUTOREPEAT_SLOW_COUNT)
        {
          RepTimer = ms2sys(AUTOREPEAT_SLOW_RATE);
          RepCnt++;
        }
        else
        {
          RepTimer = ms2sys(AUTOREPEAT_FAST_RATE);
        }
      }
    }
  }
}

//----------------------------- Set key code: --------------------------------

void Keyboard_SetCode(char c)
{
  KeyCode = c;
}

//----------------------------- Get key code: --------------------------------

char Keyboard_GetCode(void)
{
  char code = KeyCode;
  KeyCode = KEY_NO;
  return(code);
}

//----------------------------------------------------------------------------
