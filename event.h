#if !defined(_EVENT_H_)
#define _EVENT_H_

#include "hw.h"

#define EV_KEY_TOUCH    0x0000
#define EV_KEY_PRESSED  0x0200 
#define EV_KEY_LONG     0x0300
#define EV_KEY_REPEATE  0x0400
#define EV_KEY_REALIZED 0x0500
#define EV_KEY_DOUBLE   0x0600
#define EV_FUNC_FIRST   0x0700
#define EV_MASK         0x0700

extern uint16_t Event;  /* 0xKKKKAAAK */
/* AAA - it is event key. Event can be defined by & operation with EV_MASK*/
/* KKKKKK - keys - one bit fpr one key. Up to 5 keys */
volatile extern uint16_t EventQueue; /* for setting user event */


extern volatile uint8_t EvCounter; // Press delay counter
typedef void (*MenuFunction_t)(void);
extern MenuFunction_t CurrentFunction;  // Current function
#define CurrentFunc(MenuFunc) CurrentFunction = MenuFunc

extern void EventInit(void);
extern void EventKeys(void); /* This function is periodicaly called (e.g. from ISR) */
extern void EventCheck(void); /* This function should be called to handele the event */
extern void EventIdle(void);

/* Event detect delays (unit is check intervals) */
#define KEY_PRESSED_VALUE 4 // Press event delay * 5ms
#define KEY_LONG_VALUE    125  // Long press event delay * 5ms
#define KEY_REPEATE_VALUE 250 // Repeate event delay KEY_REPEATE_VALUE - KEY_LONG_VALUE - repeate interval * 5ms
#define KEY_REALIZE_VALUE 8 //  Realize event detect delay * 5ms



/* The keys must be redefined by real key mapping */
#define KEY1  GPIO_BIT(BUT3)
#define KEY2  GPIO_BIT(BUT2)
#define KEY3  GPIO_BIT(BUT1)
//#define KEY4  GPIO_IDR_13
//#define KEY5  GPIO_IDR_2
#define KEY6  GPIO_IDR_15
#define KEY_MASK_SYS (KEY1|KEY2|KEY3) /* real keys - EXTI interrupt enable mask */
#define KEY_MASK (KEY1|KEY2|KEY3|KEY6) /* real and pseudo keys. Pseudo keus can be used as other events */

#define KEY_ENTER 	KEY1
#define KEY_DOWN 		KEY2
#define KEY_UP 	    KEY3
#define KEY_F       KEY6 /* Pseudo key */

#endif /* _EVENT_H_ */
