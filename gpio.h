#if !defined(__GPIP_H__)
#define __GPIP_H__
#include "stm32f0xx.h"
// определение пина должно выглядить
//                          AINPUT  OUT10MHZ                PULL_DOWN   AF0  0
//                          INPUT   OUT2MHZ  0              PULL_UP     AF7  1
//                          OUTPUT  OUT50MHZ OUT_OPENDRAIN  0
//                          AOUTPUT
//#define PIN_NAME PORT, PIN, MODE, SPEED,   OUT_MODE,      PULL,      , AF, SET
// PORT - A, B, C, D и тд - порт ВВ
// PIN - номер бита в порте 0, 1  и т.д.
// SPEED - скорость 
// OUT_MODE - pushpull or open drain
// PULL - подтяжка
// SET - значение после инициализации

// режим
#define AINPUT (GPIO_MODER_MODER0_0|GPIO_MODER_MODER0_1)  //аналоговый вход
#define INPUT 0  // вход цифровой 
#define OUTPUT GPIO_MODER_MODER0_0 //выход
#define AOUTPUT GPIO_MODER_MODER0_1 //алтернативный выход

// скорость. Имеет смысл только для выхода. Объединяется | с режимом
#define OUT10MHZ GPIO_OSPEEDR_OSPEEDR0_0  // режим выхода среднескоростной - в поле MODE
#define OUT2MHZ  0  // низкоскоростной
#define OUT50MHZ (GPIO_OSPEEDR_OSPEEDR0_1|GPIO_OSPEEDR_OSPEEDR0_0)  // высокоскоростной

// выходной режим
#define OUT_PUSHPULL 0 // двутактный выход
#define OUT_OPENDRAIN GPIO_OTYPER_OTYPER_OT_0 // открытый сток

//подтяжка
#define PULL_DOWN GPIO_PUPDR_PUPDR0_1 // подтяжка к земле - необходимо присвоение регистра устновки-сброса
#define PULL_UP   GPIO_PUPDR_PUPDR0_0 // подтяжка к питанию

#define AF0 0x0
#define AF1 0x1
#define AF2 0x2
#define AF3 0x3
#define AF4 0x4
#define AF5 0x5
#define AF6 0x6
#define AF7 0x7


#define _TO_GPIO_2BIT(DEST_PORT, PORT, PIN, BITS) \
  ((GPIO##DEST_PORT == GPIO##PORT)*((BITS)<<((PIN)*2)))
#define _TO_GPIO_1BIT(DEST_PORT, PORT, PIN, BITS) \
  ((GPIO##DEST_PORT == GPIO##PORT)*((BITS)<<(PIN)))
#define _TO_GPIO_4LO(DEST_PORT, PORT, PIN, BITS) \
((GPIO##DEST_PORT == GPIO##PORT)*(((PIN)<8)?((BITS)<<((PIN)*4)):0))
#define _TO_GPIO_4HI(DEST_PORT, PORT, PIN, BITS) \
((GPIO##DEST_PORT == GPIO##PORT)*(((PIN)>=8)?((BITS)<<((PIN-8)*4)):0))

#define _TO_GPIO_MODER(DEST_PORT, PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) \
  _TO_GPIO_2BIT(DEST_PORT, PORT, PIN, MODE)

#define _TO_GPIO_OTYPER(DEST_PORT, PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) \
  _TO_GPIO_1BIT(DEST_PORT, PORT, PIN, OUT_MODE)

#define _TO_GPIO_OSPEEDR(DEST_PORT, PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) \
  _TO_GPIO_2BIT(DEST_PORT, PORT, PIN, SPEED)

#define _TO_GPIO_PUPDR(DEST_PORT, PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) \
  _TO_GPIO_2BIT(DEST_PORT, PORT, PIN, PULL)

#define _TO_GPIO_ODR(DEST_PORT, PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) \
  _TO_GPIO_1BIT(DEST_PORT, PORT, PIN, SET)

#define _TO_GPIO_AFRL(DEST_PORT, PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) \
  _TO_GPIO_4LO(DEST_PORT, PORT, PIN, AF)

#define _TO_GPIO_AFRH(DEST_PORT, PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) \
  _TO_GPIO_4HI(DEST_PORT, PORT, PIN, AF)

#define _GPIO_SET(PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) GPIO##PORT->BSRR = (1 << PIN)
#define _GPIO_ISSET(PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) (GPIO##PORT->IDR & (1 << PIN))
#define _GPIO_RESET(PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) GPIO##PORT->BRR = (1 << PIN)
#define _GPIO_TOGGLE(PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) GPIO##PORT->ODR ^= (1 << PIN)
#define _GPIO_BIT(PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) (1 << PIN)
#define _GPIO_PORT(PORT, PIN, MODE, SPEED, OUT_MODE, PULL, AF, SET) (GPIO##PORT)


//соотв макросы работы с портом
#define GPIO_SET(PIN_DESC) _GPIO_SET(PIN_DESC)
#define GPIO_RESET(PIN_DESC) _GPIO_RESET(PIN_DESC)
#define GPIO_TOGGLE(PIN_DESC) _GPIO_TOGGLE(PIN_DESC)
#define GPIO_ISSET(PIN_DESC) _GPIO_ISSET(PIN_DESC)
#define GPIO_BIT(PIN_DESC) _GPIO_BIT(PIN_DESC)
#define GPIO_PORT(PIN_DESC) _GPIO_PORT(PIN_DESC)

#define _GPIO_BB(PORT, PIN, MODE, OUT) (*(uint32_t*)(PERIPH_BB_BASE + ((uint32_t)&(GPIO##PORT->ODR) -  PERIPH_BASE)*32 + PIN*4))
// макрос работы с портом через bit-bang
#define GPIO_BB(PIN_DESC) _GPIO_BB(PIN_DESC)


#include "gpioau.h"

#define GPIO_CONFIG


#endif /* __GPIP_H__ */
