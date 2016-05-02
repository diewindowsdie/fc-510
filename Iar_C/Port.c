//----------------------------------------------------------------------------

//Serial port support module

//----------------------------------------------------------------------------

#include "Main.h"
#include "Port.h"
#include "Disp.h"
#include "Keyboard.h"

//----------------------------- Constants: -----------------------------------

#define BAUD 19200 //UART baud rate
#define TX_CR 16   //CR symbol index
#define TX_LF 17   //LF symbol index

#define UBRRV (int)((F_CLK * 1E6)/(16.0 * BAUD) - 0.5)

//------------------------------ Variables: ----------------------------------

static char TxPtr;         //TX buffer pointer

//------------------------- Function prototypes: -----------------------------

#pragma vector = USART_RX_vect
__interrupt void Rx_Int(void); //RX complete interrupt

//----------------------------------------------------------------------------
//--------------------------- Exported functions: ----------------------------
//----------------------------------------------------------------------------

//------------------------------- Port init: ---------------------------------

void Port_Init(void)
{
  UBRR0L = LO(UBRRV); 
  UBRR0H = HI(UBRRV);
  UCSR0B = (1 << RXCIE0) | (1 << RXEN0) | (1 << TXEN0); //RX, TX enable
  TxPtr = TX_LF + 1;   //do not TX
}

//----------------------- TX display copy via UART: --------------------------

void Port_Exe(bool t)
{
  if(t)
  {
    if(!TxPtr) UDR0 = Disp_GetChar(TxPtr++);
    else if(TxPtr <= TX_LF)
    {
      if(UCSR0A & (1 << TXC0)) 
      {
        UCSR0A = 1 << TXC0; //reset TXC flag
        if(TxPtr == TX_CR) { UDR0 = '\r'; TxPtr++; }
        else if(TxPtr == TX_LF) { UDR0 = '\n'; TxPtr++; }
        else UDR0 = Disp_GetChar(TxPtr++);
      }
    }
  }
}

//-------------------------- RX byte via UART: -------------------------------

#pragma vector = USART_RX_vect
__interrupt void Rx_Int(void)
{
  char data = UDR0;
  char code = KEY_NO;
  switch(data)
  {
  case 'M': code = KEY_MN; break; //"MENU" code
  case 'U': code = KEY_UP; break; //"UP" code
  case 'D': code = KEY_DN; break; //"DOWN" code
  case 'K': code = KEY_OK; break; //"OK" code
  case 'A': code = KEY_UD; break; //"DOWN" + "UP" ("Auto Scale") code
  case 'C': code = KEY_MK; break; //"MENU" + "OK" ("Calibrate") code
  }
  Keyboard_SetCode(code);  
}

//------------------------------ Start TX: -----------------------------------

void Port_StartTX(void)
{
  if(TxPtr == TX_LF + 1) //TX ready,
    TxPtr = 0;           //request to TX 
}

//----------------------------------------------------------------------------
