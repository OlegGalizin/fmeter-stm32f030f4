#include "gpio.h"

//                          AINPUT  OUT10MHZ                PULL_DOWN   AF0  0
//                          INPUT   OUT2MHZ  0              PULL_UP     AF7  1
//                          OUTPUT  OUT50MHZ AUOT_OPENDRAIN 0
//                          AOUTPUT
//#define PIN_NAME PORT, PIN, MODE, SPEED,   OUT_MODE,      PULL,      , AF, SET
#define LCD_CLK   A, 4, OUTPUT, OUT10MHZ, 0, 0, 0, 0
#define LCD_DA    A, 2, OUTPUT, OUT10MHZ, 0, 0, 0, 0
#define LCD_RESET A, 3, OUTPUT, OUT10MHZ, 0, 0, 0, 0
#define LCD_CE    A, 1, OUTPUT, OUT10MHZ, 0, 0, 0, 0
#define LCD_PWR   A, 0, OUTPUT, OUT10MHZ, 0, 0, 0, 0
#define SWDIO     A, 13, AOUTPUT, OUT50MHZ, 0, PULL_UP, AF0, 0
#define SWDCLK    A, 14, AOUTPUT, OUT2MHZ, 0, PULL_DOWN, AF0, 0
#define BUT1      A, 9, INPUT, 0, 0, PULL_UP, 0, 0
#define BUT2      A, 10, INPUT, 0, 0, PULL_UP, 0, 0
#define BUT3      B, 1, INPUT, 0, 0, PULL_UP, 0, 0

#define PINS SWDIO, SWDCLK, LCD_CLK, LCD_DA, LCD_RESET, LCD_CE, LCD_PWR, BUT1, BUT2, BUT3

#define MAIN_F 48000000
