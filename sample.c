#include "gpio.h"

//                          AINPUT  OUT10MHZ                PULL_DOWN   AF0  0
//                          INPUT   OUT2MHZ  0              PULL_UP     AF7  1
//                          OUTPUT  OUT50MHZ AUOT_OPENDRAIN 0
//                          AOUTPUT
//#define PIN_NAME PORT, PIN, MODE, SPEED,   OUT_MODE,      PULL,      , AF, SET
#define RELAY1 A, 4, OUTPUT, OUT10MHZ, 0, 0, 0, 0
#define RELAY2 A, 3, OUTPUT, OUT10MHZ, 0, 0, 0, 1
#define RELAY3 B, 0, OUTPUT, OUT10MHZ, 0, 0, 0, 1
#define SWDIO  A, 13, AOUTPUT, OUT50MHZ, 0, PULL_UP, AF0, 0
#define SWDCLK A, 14, AOUTPUT, OUT2MHZ, 0, PULL_DOWN, AF0, 0

#define PINS RELAY1, RELAY2, RELAY3, SWDIO, SWDCLK

static delay()
{
	volatile int i;
	for (i=0; i<500000;i++);
}

void main()
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN;
	GPIOA->MODER = TO_GPIO_MODER(A, PINS);
	GPIOA->OTYPER = TO_GPIO_OTYPER(A, PINS);
	GPIOA->OSPEEDR = TO_GPIO_OSPEEDR(A, PINS);	
	GPIOA->PUPDR = TO_GPIO_PUPDR(A, PINS);
	GPIOA->ODR = TO_GPIO_ODR(A, PINS);
	GPIOA->AFR[0] = TO_GPIO_AFRL(A, PINS);
	GPIOA->AFR[1] = TO_GPIO_AFRH(A, PINS);

	GPIOB->MODER = TO_GPIO_MODER(B, PINS);

  while(1)
	{
		GPIO_SET(RELAY1);
		delay();
		GPIO_RESET(RELAY1);
		delay();
	}
}
