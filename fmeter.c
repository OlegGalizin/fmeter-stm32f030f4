#include "hw.h"
#include "n1202.h"
//#include "event.h"

static uint16_t RefHiCounter; // hi halfword of reference counter T3
static uint16_t InHiCounter;  // hi halfword of input counter T1

static uint32_t PrevInCounter; // prev catched value T1CC1
static uint32_t PrevRefCounter;// prev catched value T3CC4

static uint32_t RefNextCheck; // end of measure interval point T3CC3
static uint32_t InCatchValue; // current catched value T1CC1


static uint32_t ResultRefCount; // Measure result
static uint32_t ResultInCount;  // Measure result
static uint8_t  ResultReady;    // Measure result ready

#define MeasureInterval 48000 /* Clock in mS */ * 1000 /* mS */

void TIM3_IRQHandler()
{
  if ( TIM3->SR & TIM_SR_CC3IF)
  {
    uint16_t RefHi = RefHiCounter;
    
    TIM3->SR &= ~TIM_SR_CC3IF;
    if ((TIM3->SR & TIM_SR_UIF) && (TIM3->CCR3 < 0x7FFF))
      RefHi++;
      
    if (RefHi == RefNextCheck>>16) // End of the measure
T1SetReastart:
    {
      uint16_t Inc = 2;
      uint16_t CurHiIn = InHiCounter;
      uint16_t CurLoIn = TIM1->CNT;
      uint32_t CurInCounter;
      
      if ((TIM1->SR & TIM_SR_UIF) && (CurLoIn < 0x7FFF))
        CurHiIn++;

      CurInCounter = (CurHiIn<<16) + CurLoIn;

      if (CurInCounter - PrevInCounter < 100) // In freq < 100 low freq. Every period is important
      {
        Inc = 1;
      }
      else if (CurInCounter - PrevInCounter > 1000000) // In freq > 1MHz
      {
        Inc = 128; // this delay is not important on hi freq
      }

      TIM1->CCMR1 = TIM_CCMR1_OC1M_2; // force low
      TIM1->CCR1 = TIM1->CNT - 1; // exclude geting capt in 64000 next tick
      TIM1->SR &= ~TIM_SR_CC1IF; // Clear compare flag
      TIM1->CCMR1 = TIM_CCMR1_OC1M_0; // Set 1 on compare

      CurHiIn = InHiCounter;
      CurLoIn = TIM1->CNT;
      if ((TIM1->SR & TIM_SR_UIF) && (CurLoIn < 0x7FFF))
        CurHiIn++;
      TIM1->CCR1 = CurLoIn + Inc; // generate compare event in some time of input signal
           
      if (Inc == 1 && TIM1->CCR1 - TIM1->CNT - 1 > 0x7FFF) // raise was skipped on Inc == 1
      {
        if (TIM1->SR & TIM_SR_CC1IF) // Very rerary event - edge of input signal in CCR aasigned time
        {
          TIM3->SR &= ~TIM_SR_CC4IF; // This inp raise can be not correct - skip it
          goto T1SetReastart;
        }
      }

      CurInCounter = CurLoIn + Inc;
      if ((TIM1->SR & TIM_SR_UIF) && (CurLoIn < 0x7FFF))
        CurHiIn++;
      CurInCounter += ((uint32_t)CurHiIn << 16);
      InCatchValue = CurInCounter;   
      RefNextCheck = RefNextCheck + MeasureInterval;
      TIM3->CCR3 = RefNextCheck;
    } // end next timeout check
  }

  if ( TIM3->SR & TIM_SR_CC4IF)
  {
    uint32_t CurRef = RefHiCounter;
    
    TIM3->SR &= ~TIM_SR_CC4IF;
    if ((TIM3->SR & TIM_SR_UIF) && (TIM3->CCR4 < 0x7FFF))
      CurRef++;
    CurRef = (((uint32_t)CurRef)<<16) + TIM3->CCR4;
    ResultRefCount = CurRef - PrevRefCounter;
    PrevRefCounter = CurRef;
    ResultInCount  = InCatchValue - PrevInCounter;
    PrevInCounter = InCatchValue;
    ResultReady = 1;
//    TIM1->CCMR1 = TIM_CCMR1_OC1M_2; //Force low level
  }
  
  if ( TIM3->SR & TIM_SR_UIF)
  {
    RefHiCounter++;
    TIM3->SR &= ~TIM_SR_UIF;
  }
}

void TIM1_BRK_UP_TRG_COM_IRQHandler()
{
  InHiCounter++;
  TIM1->SR &= ~TIM_SR_UIF;
}

void TimersInit()
{
  /* TI1 pass throuht CC1 to MMS. CC2 used with prescaler */
  TIM3->CR2 = TIM_CR2_MMS_0|TIM_CR2_MMS_1; //MMC - compare pulse
  TIM3->CCMR1 = TIM_CCMR1_CC1S_0; //|TIM_CCMR1_CC2S_1| // TI1 source for CC1
//             |TIM_CCMR1_IC2PSC_0|TIM_CCMR1_IC2PSC_1; // prescaler == 8

  /* CC4 is used for catch one of input raise throuht T1 */
  /* CC3 for time count - 48000 clocks == 1mS */
  TIM3->CCMR2 = TIM_CCMR2_CC4S_0|TIM_CCMR2_CC4S_1; // TRC source for CC4
  TIM3->ARR = 0xFFFF; 
  TIM3->CCER = TIM_CCER_CC1E|TIM_CCER_CC4E; // CC1 CC4 capture enable
  TIM3->DIER = TIM_DIER_UIE|TIM_DIER_CC4IE|TIM_DIER_CC3IE; // interrupt by update,CC4 capture, CC3 compare
  TIM3->CCR3 = MeasureInterval;
  RefNextCheck = MeasureInterval;
  TIM3->SMCR = 0|TIM_SMCR_SMS_1|TIM_SMCR_SMS_2; //source is TIM1 MMC,Enable by SMC  - some mode MUST be ON to ON SMC!

  TIM1->ARR = 0xFFFF;
  TIM1->SMCR = TIM_SMCR_TS_1|TIM_SMCR_SMS_0|TIM_SMCR_SMS_1|TIM_SMCR_SMS_2; // TIM3 MMC is source, external clock 1.
  TIM1->DIER =  TIM_DIER_UIE; // interrupts by update
  TIM1->CCMR1 = TIM_CCMR1_OC1M_2;//Force low level
  TIM1->CR2 = TIM_CR2_MMS_2; // OC1REF as source for MMC
  
//  TIM1->CCER = TIM_CCER_CC1NE; // Out CC1N to PA7
//  TIM1->BDTR = TIM_BDTR_MOE; // All outputs enable
  
  
  NVIC_EnableIRQ(TIM3_IRQn); /* Timer IRQ */
  NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn); /* Timer IRQ */

  TIM1->CR1 |= TIM_CR1_CEN;
  TIM3->CR1 |= TIM_CR1_CEN;
  
  TIM14->ARR = 48-1; // test signal 1 MHz
  TIM14->CCR1 = 23;
  TIM14->CCMR1 = TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1M_2; //pwm 1 at PWMOUT (PA7)
  TIM14->CCER = TIM_CCER_CC1E;
  TIM14->CR1 |= TIM_CR1_CEN;
}


void OutValue(uint8_t Y, uint8_t X, uint32_t Num, uint8_t DotPosition, uint8_t SelectPos)
{
  int i;
  int Div = 100000;
  uint8_t DisplayFlag = 0;
  
  for(i=0; i<6; i++)
  {
    char Chr;
    uint32_t Light = 0;

    if (i == DotPosition )
    {
      LcdChr ( Y_POSITION*(Y)+X_POSITION*X+MUL2+1, "." );
      X=X+2;
      DisplayFlag++;
    }

    if ( DisplayFlag == 0 && i == (DotPosition - 1))
    {
      DisplayFlag++;
    }

    Chr = Num / Div;
    if ( DisplayFlag == 0 && Chr == 0 && i < 5)
      Chr = ' ';
    else
    {
      DisplayFlag = 1;
      Chr = Chr + '0';
    }
    if (i == SelectPos)
    {
      Light = INVERSE;
    } 
    LcdChr (Light + Y_POSITION*Y+X_POSITION*X+MUL2+1, &Chr );
    X = X + 2;
    Num = Num % Div;
    Div = Div / 10;
  }
//  if ( DotPosition == 4)
//  {
//    LcdChr ( Y_POSITION*(Y)+X_POSITION*X+1, " " );
//    LcdChr ( Y_POSITION*(Y+1)+X_POSITION*X+1, " " );
//  }
}


int main()
{
//  CurrentFunc(StartFunction);

  RCC->AHBENR |= RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN;
  RCC->APB2ENR = RCC_APB2ENR_SYSCFGEN|RCC_APB2ENR_TIM17EN|RCC_APB2ENR_TIM1EN|RCC_APB2ENR_DBGMCUEN;
  RCC->APB1ENR = RCC_APB1ENR_TIM3EN|RCC_APB1ENR_TIM14EN;
  GPIOA->MODER = TO_GPIO_MODER(A, PINS);
  GPIOA->OTYPER = TO_GPIO_OTYPER(A, PINS);
  GPIOA->OSPEEDR = TO_GPIO_OSPEEDR(A, PINS);  
  GPIOA->PUPDR = TO_GPIO_PUPDR(A, PINS);
  GPIOA->ODR = TO_GPIO_ODR(A, PINS);
  GPIOA->AFR[0] = TO_GPIO_AFRL(A, PINS);
  GPIOA->AFR[1] = TO_GPIO_AFRH(A, PINS);
  GPIOB->MODER = TO_GPIO_MODER(B, PINS);
  GPIOB->PUPDR = TO_GPIO_PUPDR(B, PINS);

  DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM3_STOP;
  DBGMCU->APB2FZ |= DBGMCU_APB2_FZ_DBG_TIM1_STOP|DBGMCU_APB1_FZ_DBG_TIM14_STOP;

  
  RCC->CR |= RCC_CR_HSEON;
  while ( (RCC->CR & RCC_CR_HSERDY) == 0)
    ; /* BLANK */

  FLASH->ACR |= FLASH_ACR_LATENCY|FLASH_ACR_PRFTBE;// 1 wait state + prefetch
  
  RCC->CFGR = RCC_CFGR_PLLMUL_2|RCC_CFGR_PLLSRC ; // pll x6  HSE
  RCC->CR |= RCC_CR_PLLON;
  while((RCC->CR & RCC_CR_PLLRDY) == 0)
    ; /* BLANK */
  RCC->CFGR |= RCC_CFGR_SW_1; // switch to pll
  
  GPIO_SET(LCD_PWR);

  LcdInit();
  LcdClear();
//  EventInit();

  TimersInit();  
  do
  {
  	if (ResultReady)
    { 
      volatile double Result;
      uint32_t IntRes;
      
      ResultReady = 0;
      Result = (double)ResultInCount * (double)MAIN_F * (double)1000 / (double)ResultRefCount;
      IntRes = Result/1000000;     
//      if ( IntRes > 0)
      {
        OutValue(0, 0, IntRes, 20, 20);
        LcdChr(14*X_POSITION + Y_POSITION*0 + MUL2 + 1, "k");
      }
      IntRes = ((uint32_t)Result)%1000000;     
      OutValue(3, 0, IntRes, 3, 20);
//      LcdChr(12*X_POSITION + Y_POSITION*3 + MUL2 + 3, "Hz");
      
    }
//    __WFI(); // It decreases power but turn off the SWD!!!!
  }
  while(1);
}
