//----------------------------------------------------------------------------

//Level meter support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Meter.h"
#include "Lcd.h"
#include "Disp.h"

//----------------------------- Constants: -----------------------------------

#define MUX_INP     6   //ADC channel 6 (main input level)
#define MUX_PRE     7   //ADC channel 7 (prescaler input level)
#define ADC_RES   255   //ADC resolution
#define BAR_INT   100   //level bar integration time, ms
#define BAR_DEC  1000   //level bar decay time, ms
#ifdef DIG_DISPLAY
  #define BAR_LNG   8   //level bar length, chars
#else
  #define BAR_LNG  14   //level bar length, chars
#endif
#define BAR_BPC     3   //level bar density, bars per char
#define BAR_0       1   //first bar symbol code

#define BAR_BRS (BAR_LNG * BAR_BPC)  //total bars count
#define BAR_STP (BAR_BRS * BAR_INT / BAR_DEC) //bar decay step
#define BAR_FIR ((int)(BAR_INT * 1E3 / T_SYS)) //level bar FIR points

#ifdef DIG_DISPLAY
  #define DIG_INT   300   //digital level integration time, ms
  #define DIG_SLP    20   //25   //digital level slope, mV/dBm
  #define DIG_REF  1770   //2100   //digital level reference point, mV at 0 dBm
  #define V_REF    2560   //reference voltage, mV

  #define DIG_FIR (DIG_INT / BAR_INT) //level digs FIR points
#endif

#define ADCSRA_VAL ((1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0))
#define ADMUX_VAL ((1 << REFS1) | (1 << REFS0) | (1 << ADLAR))
#define ADC_START (1 << ADSC)

//------------------------------ Variables: ----------------------------------

static char BarFilter;    //bar filter points counter
static unsigned int AdcCode; //ADC code
static char BarPos;       //current bar position
static bool BarUpdated;   //bar update flag
#ifdef DIG_DISPLAY
  static char DigFilter;  //digit filter points counter
  static unsigned int DigCode; //code for digital level display
  static char DigVal;     //value for digital level display
#endif

//----------------------------------------------------------------------------
//--------------------------- Exported functions: ----------------------------
//----------------------------------------------------------------------------

//------------------------------ Meter init: ---------------------------------

void Meter_Init(void)
{
  ADCSRA = ADCSRA_VAL;
  BarFilter = BAR_FIR;
  AdcCode = 0;
  BarUpdated = 0;
#ifdef DIG_DISPLAY
  DigFilter = DIG_FIR;
  DigCode = 0;
#endif
}

//----------------------------- Measure level: -------------------------------

void Meter_Exe(bool t)
{
  if(t)
  {
    if(BarFilter)
    {
      AdcCode += ADCH;     //read and accumulate ADC.9..ADC.2
      BarFilter--;         //next point
    }
    else
    {
      char bar = AdcCode / BAR_FIR * BAR_BRS / ADC_RES;
      if(BarPos > bar + BAR_STP) BarPos = BarPos - BAR_STP;
        else BarPos = bar;
#ifdef DIG_DISPLAY
      if(DigFilter)
      {
        DigCode += AdcCode / BAR_FIR;
        DigFilter--;
      }
      else
      {
        DigVal = DigCode / DIG_FIR;
        DigFilter = DIG_FIR;
        DigCode = 0;
      }
#endif
      BarUpdated = 1;
      BarFilter = BAR_FIR;  //new cycle
      AdcCode = 0;
    }
    ADMUX = ADMUX_VAL | (Pin_FDIV? MUX_PRE : MUX_INP);
    ADCSRA |= ADC_START;
  }
}

//--------------------------- Clear meter line: ------------------------------

void Meter_Clear(void)
{
  LCD_WrCmd(LINE2 + 0); //line = 2, pos = 1
  for(char i = 0; i < LCD_SIZE; i++)
    LCD_WrData(' ');
}

//-------------------------- Check meter update: -----------------------------

bool Meter_Updated(void)
{
  if(BarUpdated)
  {
    BarUpdated = 0;
    return(1);
  }
  return(0);
}

//-------------------------- Display meter line: -----------------------------

void Meter_Display(void)
{
  LCD_WrCmd(LINE2 + 0); //line = 2, pos = 1
#ifdef DIG_DISPLAY
  //digital display:
  int v = (long)DigVal * V_REF / ADC_RES;
  signed char dB = (v - DIG_REF) / DIG_SLP;
  if(dB < 0)
  {
    LCD_WrData('-');
    dB = -dB;
  }
  else
  {
    LCD_WrData('+');
  }
  char d1 = dB / 10;
  char d2 = dB % 10;
  LCD_WrData(d1? (d1 + 0x30) : ' ');
  LCD_WrData(d2 + 0x30);
  LCD_WrData(' ');
  LCD_WrData('d');
  LCD_WrData('B');
  LCD_WrData('m');
  LCD_WrData(' ');
#else
  LCD_WrData('L');
  LCD_WrData(' ');
#endif
  //bar display:
  char f = BarPos / BAR_BPC;
  char t = BarPos % BAR_BPC;
  for(char n = 0; n < BAR_LNG; n++)
  {
    if(n < f) LCD_WrData(BAR_0 + BAR_BPC);
      else if(n == f) LCD_WrData(BAR_0 + t);
        else LCD_WrData(BAR_0);
  }
}

//----------------------------------------------------------------------------
