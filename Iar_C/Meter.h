//----------------------------------------------------------------------------

//Level meter support module: header file

//----------------------------------------------------------------------------

#ifndef MeterH
#define MeterH

//----------------------------- Constants: -----------------------------------

#define LINE2 0xC0           //line 2 code for 1602 LCD

//------------------------- Function prototypes: -----------------------------

void Meter_Init(void);       //level meter init
void Meter_Exe(bool t);      //level measure
void Meter_Clear(void);      //clear meter line
bool Meter_Updated(void);    //check meter update
void Meter_Display(void);    //display meter line

//----------------------------------------------------------------------------

#endif
