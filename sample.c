#include "hw.h"
#include "n1202.h"

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

  GPIO_SET(LCD_PWR);
  LcdInit();
	LcdClear();
	LcdChr(X_POSITION*3+ Y_POSITION*4 + INVERSE+11, "Hello world");
}
