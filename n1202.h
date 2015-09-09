#if !defined(_N1202_H_)
#define _N1202_H_
#include "hw.h"

static void LCD_DELAY(void) 
{ 
	int i=MAIN_F/6000000/3+1; 
  while(i) 
  {
    __NOP();
    i--;
	}; 
}
static void LCD_BIG_DELAY(void)
{
	int i=MAIN_F/100/3+1; 
  while(i) 
  {
    __NOP();
    i--;
	}; 
}

#if defined(LCD_USART)  
#define LCD_CE_PIN_HIGHT  while ( (USART1->SR & USART_SR_TC) == 0) /* BLANK */; \
    LCD_DELAY(); GPIO_SET(LCD_CE)
#define LCD_CE_PIN_LOW    GPIO_RESET(LCD_CE)
#else
#define LCD_CE_PIN_LOW    GPIO_RESET(LCD_CE); LCD_DELAY()
#define LCD_CE_PIN_HIGHT  GPIO_SET(LCD_CE)
#endif



void LcdSend    ( uint8_t data, uint8_t Flags );
#define LCD_DATA    1
#define LCD_COMMAND 0
#define LCD_SET_CE  2 /* Cristal Enable before the command */
#define LCD_RESET_CE 4 /* Cristal disable after the command */

/*--------------------------------------------------------------------------------------------------
                                 Public function prototypes
--------------------------------------------------------------------------------------------------*/
void LcdInit       ( void );
#define X_OFFSET   0x00010000 /* Offset in dot */
#define X_POSITION (0x00010000*6) /* Offset in 6 dot chars */
#define Y_POSITION 0x01000000 /* in 8 dot chars */
#define MUL2       0x40000000 /* double size */
#define MUL3       0x80000000 /* triple size */
#define MUL4       0xC0000000 /* X4 */
#define INVERSE    0x20000000 /* Inverse bit */
/*--------------------------------------------------------------------------------------------------

  Name         :  LcdChr
  Argument(s)  :  Ctrl = X*X_POSITION+y*Y_POSITION+ MUL2 + OutLen + INVERSE
                  Str - out string
 Description  :  Displays a character at current cursor location and increment cursor location.
              It is parts of Ctrl. Ctrl = X_POSITION*X+Y_POSITION*Y+LENGTH+INVERSE+MUL2
              Length should be >= strlen(Str). 
              Example:LcdChr(5*X_POSITION+3*Y_POSITION+8, "123") - will out string 123 in 
              position 6 row 4 8 chars len.
              The 5 symbols after 123 will be cleaned ( 8 - strlen("123"))
  Return value :  None.

--------------------------------------------------------------------------------------------------*/
void LcdChr ( uint32_t Ctrl, const char* Str );
/*  LcdChr ( Y_POSITION*2+X_POSITION*3+2+MUL2+INVERSE, "34" );  */

void LcdClear(void);
void LcdContrast ( uint8_t Contrast );
void LcdGotoXY(uint8_t X, uint8_t Y);

#define Mju ('[' + 1)
#define Omega ('?' + 1)

#endif  //  _N1202_H_
/*--------------------------------------------------------------------------------------------------
                                         End of file.
--------------------------------------------------------------------------------------------------*/
