//----------------------------------------------------------------------------

//Frequency Counter FC-510
//Menu implementation module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Menu.h"
#include "Keyboard.h"
#include "Disp.h"
#include "Port.h"
#include "Sound.h"
#include "Count.h"
#ifdef LCD1602
  #include "Meter.h"
  #include "Lcd.h"
#endif

//----------------------------- Constants: -----------------------------------

#define SIGNATURE 0xBEDE //EEPROM signature
#define T_SPLASH    2500 //splash screen indication delay, ms
#define T_CALIB      300 //calibration update period, ms
#define T_AUTO      2000 //auto scale indication time, ms
#define T_BLINK      250 //display blink time, ms
#define AUTO_SCALE  0x80 //auto scale flag
#define SCALE_NOM      5 //nom scale

enum
{
  MNU_NO,     //no menu code
  MNU_SPLASH, //splash screen menu code
  MNU_MAIN,   //main menu code
  MNU_AUTO,   //auto scale indication menu
  MNU_SETUP,  //setup menu code
};

enum
{
  PAR_MODE, //mode parameter index
  PAR_GATE, //gate parameter index
  PAR_AVG,  //average parameter index
  PAR_IF,   //IF parameter index
  PAR_PRE,  //prescaler parameter index
  PAR_INT,  //interpolator parameter index
  PAR_RF,   //RF parameter index
  PAR_SIF,  //IF step parameter index
  PAR_SRF,  //RF step parameter index
  PARAMS    //params count
};

enum
{ P_MIN,    //min value
  P_NOM,    //nom value
  P_MAX,    //max value
  LIMS
};

const __flash long ParLim[PARAMS][LIMS] =
{
  {        0,    MODE_F, MODES - 1 }, //PAR_MODE
  {        1,      1000,     10000 }, //PAR_GATE
  {        1,         1,       100 }, //PAR_AVG
  {  -999999,         0,    999999 }, //PAR_IF
  {        1,         1,      1000 }, //PAR_PRE
  {        0,         1,         1 }, //PAR_INT
  { 10000000, 128000000, 999999999 }, //PAR_RF
  {        1,        10,    100000 }, //PAR_SIF
  {        1,         1, 100000000 }, //PAR_SRF
};

//----------------------------- Variables: -----------------------------------

static char Menu;        //active menu index
static char DispMenu;    //displayed menu
static int  MenuTimer;   //menu delay timer
static char Param;       //current editing param index
static char KeyCode;     //last pressed key code
static bool Repeat;      //repeat flag
static bool Hold;        //display hold flag
static bool Hide;        //display hide flag
static char Scale;       //current value scale
static long Par[PARAMS]; //params array

__no_init __eeprom int  ESignature; //EEPROM signature
__no_init __eeprom long EPar[PARAMS];
__no_init __eeprom char EScale[MODES]; //Scales in EEPROM

//------------------------- Function prototypes: -----------------------------

void Mnu_Splash(bool ini);    //splash screen menu
void Mnu_Main(bool ini);      //main menu
void Mnu_Auto(bool ini);      //auto scale indication menu
void Mnu_Setup(bool ini);     //setup menu

void ParToEEPROM(void);       //save params to the EEPROM
void Show_Main(char n);       //show main menu
void Show_Setup(char m);      //show setup menu
bool MoveDP(char key);        //move DP
bool ParUpDn(char m, bool dir); //param step up/down
void SetupCounter(void);      //send params to counter

//------------------------------ Menu init: ----------------------------------

void Menu_Init(void)
{
  Disp_Init();                //display init
  Port_Init();                //port init
  Keyboard_Init();            //keyboard init
#ifdef LCD1602
  Meter_Init();               //level meter init
#endif

  if(ESignature != SIGNATURE) //check EEPROM signature, if error, init params:
  {
    for(char i = 0; i < PARAMS; i++)
      Par[i] = ParLim[i][P_NOM];
    ParToEEPROM();                        //params init
    for(char i = 0; i < MODES; i++)
      EScale[i] = SCALE_NOM + AUTO_SCALE; //scales init
  }
  else
  {
    for(char i = 0; i < PARAMS; i++)
      Par[i] = EPar[i];                   //read params from EEPROM
  }
  SetupCounter();
  Count_Start();              //start counter

  DispMenu = MNU_NO;          //no menu
  Hold = 0;                   //no hold mode
  Hide = 0;                   //display not hided
  Menu = MNU_SPLASH;          //go to splash screen menu
}

//----------------------------- Menu execute: --------------------------------

void Menu_Exe(bool t)
{
  Port_Exe(t);                    //TX display copy via UART
  Keyboard_Exe(t);                //scan keyboard
#ifdef LCD1602
  Meter_Exe(t);                   //level measure
#endif

  if(t)
  {
    if(MenuTimer) MenuTimer--;    //menu timer processing
  }

  KeyCode = Keyboard_GetCode();   //read key code
  Repeat = KeyCode & REP_R;       //set/clear repeat flag
  KeyCode &= ~REP_R;              //clear repeat flag in key code
  if(KeyCode && !Repeat)
    Sound_Beep();                 //beep if not repeat

  bool MnuIni = Menu != DispMenu; //need menu draw flag
  switch(Menu)                    //execute menus
  {
  case MNU_SPLASH:  Mnu_Splash(MnuIni); break; //splash screen menu
  case MNU_MAIN:    Mnu_Main(MnuIni);   break; //main menu
  case MNU_AUTO:    Mnu_Auto(MnuIni);   break; //auto scale menu
  case MNU_SETUP:   Mnu_Setup(MnuIni);  break; //setup menu
  }

  if(!Repeat && (KeyCode != KEY_NO)) //if key not processed
    Sound_Bell();                    //error bell
}

//----------------------------------------------------------------------------
//------------------------- Splash screen menu: ------------------------------
//----------------------------------------------------------------------------

void Mnu_Splash(bool ini)
{
  static char __flash Mnu_Str[] = "FC-510";
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Sound_Beep();                //initial beep
    Disp_SetPos(LCD_SIZE / 2 - 2);
    Disp_PutString(Mnu_Str);     //show menu text
#ifdef LCD1602
    LCD_WrCmd(LINE2 + 6); //line = 2, pos = 7
    LCD_WrData('V');
    LCD_WrData((char)VERSION + 0x30);
    LCD_WrData('.');
    LCD_WrData((char)(VERSION * 10) % 10 + 0x30);
#endif
    Disp_Update();               //display update
    MenuTimer = ms2sys(T_SPLASH); //load menu timer
    DispMenu = Menu;             //menu displayed
  }
  //check any key press:
  if(KeyCode != KEY_NO)          //any key
  {
    MenuTimer = 0;               //timer clear
    KeyCode = KEY_NO;            //key code processed
  }
  //menu timer check:
  if(!MenuTimer)                 //timer overflow,
  {
    Sound_Beep();                //beep
#ifdef LCD1602
    Meter_Clear();
#endif
    Menu = MNU_MAIN;             //go to main menu
  }
}

//----------------------------------------------------------------------------
//----------------------------- Main menu: -----------------------------------
//----------------------------------------------------------------------------

void Mnu_Main(bool ini)
{
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Show_Main(Par[PAR_MODE]);    //draw main menu
    MenuTimer = ms2sys(T_BLINK); //load blink timer
    DispMenu = Menu;             //menu displayed
  }
  //blink display in hold mode:
  if(Hold && !MenuTimer)
  {
    Hide = !Hide;
    DispMenu = MNU_NO;           //redraw menu
  }
  //display measured value:
  if(Count_Ready())              //check counter ready
  {
    DispMenu = MNU_NO;           //redraw menu
  }
#ifdef LCD1602
  if(Meter_Updated())
  {
    Meter_Display();             //display meter line
  }
#endif
  //MENU key:
  if(KeyCode == KEY_MN)
  {
    if(Hold)
    {
      Hold = 0;                  //off hold mode
      Hide = 0;                  //show display
      Count_Start();             //start counter (no stat. clear!)
      DispMenu = MNU_NO;         //redraw menu
    }
    else
    {
#ifdef LCD1602
      Meter_Clear();
#endif
      Param = PAR_MODE;          //param index
      Menu = MNU_SETUP;          //go to setup menu
    }
    KeyCode = KEY_NO;            //key code processed
  }
  //MENU + OK key:
  if(KeyCode == KEY_MK)
  {
#ifdef LCD1602
    Meter_Clear();
#endif
    Param = PAR_RF;              //param index (calibrate)
    Menu = MNU_SETUP;            //go to setup menu
    KeyCode = KEY_NO;            //key code processed
  }
  //OK key:
  if(KeyCode == KEY_OK)
  {
    Hold = !Hold;                //on/off hold mode
    Hide = Hold;                 //hide display value if hold
    if(Hold)
    {
      Count_Stop();              //stop counter if hold
    }
    else
    {
      Count_ClearStat();         //clear statistics
      Count_Start();             //start counter
    }
    DispMenu = MNU_NO;           //redraw menu
    KeyCode = KEY_NO;            //key code processed
  }
  //UP/DOWN key:
  if(KeyCode != KEY_NO)
  {
    if(MoveDP(KeyCode))          //shift DP
    {
      Count_SetScale(Scale);     //set new scale
      EScale[Par[PAR_MODE]] = Scale; //save scale to EEPROM
      DispMenu = MNU_NO;         //redraw menu
      if(KeyCode == KEY_UD)      //UP + DOWN pressed,
        Menu = MNU_AUTO;         //go to auto scale menu
      KeyCode = KEY_NO;          //key code processed
    }
  }
}

//----------------------------------------------------------------------------
//------------------------------ Menu Auto: ----------------------------------
//----------------------------------------------------------------------------

void Mnu_Auto(bool ini)
{
  static char __flash Str_Auto1[] = "Auto On";
  static char __flash Str_Auto0[] = "Auto Off";
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Disp_Clear();                //clear display
    Disp_SetPos(LCD_SIZE / 2 - 3);
    if(Scale & AUTO_SCALE)       //show menu text
      Disp_PutString(Str_Auto1);
        else Disp_PutString(Str_Auto0);
    Disp_Update();               //display update
    MenuTimer = ms2sys(T_AUTO);  //load menu timer
    DispMenu = Menu;             //menu displayed
  }
  //block keys:
  KeyCode = KEY_NO;              //key code processed
  //check timer:
  if(!MenuTimer)                 //timer overflow,
  {
    Sound_Beep();                //beep
    Menu = MNU_MAIN;             //go to main menu
  }
}

//----------------------------------------------------------------------------
//----------------------------- Setup menu: ----------------------------------
//----------------------------------------------------------------------------

void Mnu_Setup(bool ini)
{
  //draw menu:
  if(ini)                        //if redraw needed
  {
    Count_Stop();                //stop counter
    Show_Setup(Param);           //draw setup menu
    if(Param == PAR_INT)         //if interpolator par,
    {
      Count_StartCalib();        //start interpolator calibration
      MenuTimer = ms2sys(T_CALIB);             //reload timer
    }
    DispMenu = Menu;             //menu displayed
  }
  //update interpolator value:
  if(Param == PAR_INT && !MenuTimer)
  {
    DispMenu = MNU_NO;           //redraw menu
  }
  //MENU key:
  if(KeyCode == KEY_MN)
  {
    if(Param >= PAR_INT)
    {
      KeyCode = KEY_OK;          //last param, exit
    }
    else
    {
      Param++;                   //new param
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
    }
  }
  //DOWN key:
  if(KeyCode == KEY_DN)
  {
    if(ParUpDn(Param, 0))          //step param down
    {
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
    }
  }
  //UP key:
  if(KeyCode == KEY_UP)
  {
    if(ParUpDn(Param, 1))          //step param up
    {
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
    }
  }
  //OK key:
  if(KeyCode == KEY_OK)
  {
    if(Param == PAR_SIF || Param == PAR_SRF)
    {
      KeyCode = KEY_UD;          //key code processed
    }
    else
    {
      ParToEEPROM();             //save parameters to EEPROM
      SetupCounter();
      Count_Start();             //start counter
      Menu = MNU_MAIN;           //go to main menu
      KeyCode = KEY_NO;          //key code processed
    }
  }
  //MENU + OK key:
  if(KeyCode == KEY_MK)
  {
    Par[Param] = ParLim[Param][P_NOM];
    DispMenu = MNU_NO;           //redraw menu request
    KeyCode = KEY_NO;            //key code processed
  }
  //UP + DOWN key:
  if(KeyCode == KEY_UD)          //UP + DOWN pressed,
  {
    if(Param == PAR_IF)
    {
      Param = PAR_SIF;
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
      return;
    }
    if(Param == PAR_RF)
    {
      Param = PAR_SRF;
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
      return;
    }
    if(Param == PAR_SIF)
    {
      Param = PAR_IF;
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
      return;
    }
    if(Param == PAR_SRF)
    {
      Param = PAR_RF;
      DispMenu = MNU_NO;         //redraw menu request
      KeyCode = KEY_NO;          //key code processed
      return;
    }
  }
}

//----------------------------------------------------------------------------
//------------------------ Additional functions: -----------------------------
//----------------------------------------------------------------------------

//--------------------------- Show main menu: --------------------------------

static __flash char Str_V[MODES][4] =
{
  "F  ", //frequency
  "FIF", //frequency ± IF
  "P  ", //period
  "HI ", //high level duration
  "LO ", //low lewel duration
  "D  ", //duty cycle
  "Rot", //RPM
  "FH ", //maximum frequency
  "FL ", //minimum frequency
  "dF "  //variation of frequency
};

void Show_Main(char n)
{
  long min, max; char s;
  Disp_Clear();                       //clear display

  //blink name:
  if(!Hide)
  {
    if(n == MODE_FIF)
      Disp_PutChar('f');                //display "f"
        else Disp_PutString(Str_V[n]);  //show value name
  }

  long v = Count_GetValue();          //read counter
  if(Count_Ready()) Count_Start();    //start counter

  char p = (Scale & ~AUTO_SCALE) + 1; //DP position
  switch(n)
  {
#ifdef HI_RES
  case MODE_F:
  case MODE_FIF:
  case MODE_P:
  case MODE_D:
    s = 3;
    min = 100000000L;
    max = 999999999L;
    break;
  case MODE_R:
    s = 5;
    min = 1000000L;
    max = 9999999L;
    break;
  default:
    s = 4;                            //first position
    min = 10000000L;                  //min value for auto scale
    max = 99999999L;                  //max value for auto scale
#else
  case MODE_F:
  case MODE_FIF:
  case MODE_P:
  case MODE_D:
    s = 3;
    min = 10000000L;
    max = 99999999L;
    break;
  case MODE_R:
    s = 5;
    min = 100000L;
    max = 999999L;
    break;
  default:
    s = 4;                            //first position
    min = 1000000L;                   //min value for auto scale
    max = 9999999L;                   //max value for auto scale
#endif
  }

  //blink value:
  //if(!Hide)
    Disp_Val(s, p, v);                //show value
  Disp_Update();                      //update display

  if(Scale & AUTO_SCALE)              //if auto scale
  {
    if(v >= 0)
    {
      if(v < min) MoveDP(KEY_DN);     //move DP left
      if(v > max) MoveDP(KEY_UP);     //move DP right
    }
    else
    {
      if(v > -min) MoveDP(KEY_DN);    //move DP left
      if(v < -max) MoveDP(KEY_UP);    //move DP right
    }
    Scale |= AUTO_SCALE;              //restore auto scale flag
    Count_SetScale(Scale);            //update scale
  }
}

//-------------------------- Show setup menu: --------------------------------

static __flash char Str_P[PARAMS][5] =
{
  "Ind ", //indication mode
  "Gate", //gate
  "Avg ", //average
  "IF  ", //IF frequency
  "Pre ", //prescaler ratio
  "Int ", //interpolator on/off
  "C   ", //calibration Fref
  "S   ", //IF step
  "S   "  //Fref step
};

static __flash char Str_On[4] = "On ";
static __flash char Str_Off[4] = "Off";

void Show_Setup(char m)
{
  Disp_Clear();                     //clear display
  Disp_PutString(Str_P[m]);         //show param name
  long v = Par[m];

  switch(m)
  {
  case PAR_GATE:
  case PAR_AVG:
  case PAR_PRE:
    Disp_Val(6, 0, v);              //show Gate value
    break;
  case PAR_IF:
  case PAR_SIF:
    Disp_Val(5, 9, v);              //show IF value
    break;
  case PAR_RF:
  case PAR_SRF:
    Disp_Val(2, 6, v);              //show Fref value
    break;
  case PAR_MODE:
    Disp_SetPos(5);                 //set display position
    Disp_PutString(Str_V[(char)v]); //show value name
    break;
  case PAR_INT:
    Disp_SetPos(5);                   //set display position
    if((char)v)                       //show interpolator state
    {
      Disp_PutString(Str_On);
      Disp_Val(8, 0, Count_GetCalib()); //show interpolator calibration value
    }
    else
    {
      Disp_PutString(Str_Off);
    }
    break;
  }
  Disp_Update();                      //update display
}

//------------------------------- Move DP: -----------------------------------

bool MoveDP(char key)
{
  char s = Scale & ~AUTO_SCALE;       //scale code
  bool a = Scale & AUTO_SCALE;        //auto scale flag

  if(key == KEY_DN) { a = 0; s--; }   //scale left
  if(key == KEY_UP) { a = 0; s++; }   //scale right
  if(key == KEY_UD) { a = !a; }       //invert auto scale flag

  if(s > MAX_SCALE) s = MAX_SCALE;    //limit scale code in top
  switch((char)Par[PAR_MODE])         //limit scale code in bot
  {
#ifdef HI_RES
  case MODE_F:
  case MODE_FIF:
  case MODE_P:
  case MODE_D:
    if(s < 1) s = 1; break;
  case MODE_R:
    if(s < 3) s = 3; break;
  default:
    if(s < 2) s = 2;
#else
  case MODE_F:
  case MODE_FIF:
  case MODE_P:
  case MODE_D:
    if(s < 2) s = 2; break;
  case MODE_R:
    if(s < 4) s = 4; break;
  default:
    if(s < 3) s = 3;
#endif
  }
  char sc = s | (a? AUTO_SCALE : 0);  //combine scale code and auto flag
  if(Scale == sc) return(0);          //no changes, return
  Scale = sc;                         //save new scale value
  return(1);
}

//---------------------------- Change param: --------------------------------

bool ParUpDn(char m, bool dir)
{
  long v = Par[m];
  long s = 1;
  long Min = ParLim[m][P_MIN];
  long Max = ParLim[m][P_MAX];
  if(m == PAR_IF) s = Par[PAR_SIF];
  if(m == PAR_RF) s = Par[PAR_SRF];
  if(!dir) s = -s;
  if((dir && v < Max) || (!dir && v > Min))
  {
    if(m == PAR_GATE || m == PAR_AVG)
    {
      int n = v;
      if(dir)
      {
        if(n == 2 || n == 20 || n == 200 || n == 2000)
          n = (n * 5) / 2;  // * 2.5
            else n = n * 2; // * 2
      }
      else
      {
        if(n == 5 || n == 50 || n == 500 || n == 5000)
          n = (n * 2) / 5;  // / 2.5
            else n = n / 2; // / 2
      }
      v = n;
    }
    else if(m == PAR_SIF || m == PAR_SRF)
    {
      v = (dir? (v * 10) : (v / 10));
    }
    else
    {
      v = v - v % s + s;
      if(v > Max) v = Max;
      if(v < Min) v = Min;
    }
    Par[m] = v;
    return(1);
  }
  return(0);
}

//---------------------- Save params to the EEPROM: --------------------------

void ParToEEPROM(void)
{
  for(char i = 0; i < PARAMS; i++)
    if(EPar[i] != Par[i]) EPar[i] = Par[i];
  if(ESignature != SIGNATURE) ESignature = SIGNATURE;
}

//----------------------- Set counter params: --------------------------------

void SetupCounter(void)
{
  Count_SetMode(Par[PAR_MODE]);  //set counter mode
  Count_SetGate(Par[PAR_GATE]);  //set gate time
  Count_SetAvg(Par[PAR_AVG]);    //set number of averages
  Count_SetIF(Par[PAR_IF]);      //set IF
  Count_SetPre(Par[PAR_PRE]);    //set prescaler ratio
  Count_SetInt(Par[PAR_INT]);    //interpolator enable/disable
  Count_SetFref(Par[PAR_RF]);    //set Fref

  Scale = EScale[Par[PAR_MODE]];
  Count_SetScale(Scale);         //set scale
}

//----------------------------------------------------------------------------


