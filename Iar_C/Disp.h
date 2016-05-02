//----------------------------------------------------------------------------

//Display support module: header file

//----------------------------------------------------------------------------

#ifndef DispH
#define DispH

//----------------------------- Constants: -----------------------------------

#ifdef LCD16XX
  #define LCD_SIZE 16       //size of LCD16XX
#else
  #define LCD_SIZE 10       //size of LCD10
#endif
#define POINT 0x80          //decimal point

//------------------------- Function prototypes: -----------------------------

void Disp_Init(void);       //display init
void Disp_Update(void);     //copy display memory to LCD
void Disp_Clear(void);      //display clear and set first position
void Disp_SetPos(char p);   //set display position
void Disp_PutChar(char ch); //display char
void Disp_PutString(char __flash *s);  //display string
void Disp_Val(char s, char p, long t); //display value
char Disp_GetChar(char n);  //get char from display buffer

//----------------------------------------------------------------------------

#endif
