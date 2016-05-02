//----------------------------------------------------------------------------

//Keyboard support module: header file

//----------------------------------------------------------------------------

#ifndef KeyboardH
#define KeyboardH

//----------------------------- Constants: -----------------------------------

#define KEY_NO  0x00  //no key pressed
#define KEY_MN  0x01  //key "MENU"
#define KEY_DN  0x02  //key "DOWN"
#define KEY_UP  0x04  //key "UP"
#define KEY_UD  0x06  //key "UP" + "DOWN"
#define KEY_OK  0x08  //key "OK"
#define KEY_MK  0x09  //key "MENU" + "OK"

#define REP_R   0x80  //autorepeat

//-------------------------- Прототипы функций: ------------------------------

void Keyboard_Init(void);      //keyboard module init
char Keyboard_Scan(void);      //get scan code
void Keyboard_Exe(bool t);     //process keyboard
void Keyboard_SetCode(char c); //set key code
char Keyboard_GetCode(void);   //get key code

//----------------------------------------------------------------------------

#endif

