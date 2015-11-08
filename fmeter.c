#include "hw.h"
#include "n1202.h"
#include "event.h"

static uint16_t RefHiCounter; // hi halfword of reference counter T3
static uint16_t InHiCounter;  // hi halfword of input counter T1

static uint32_t PrevInCounter; // prev catched value T1CC1
static uint32_t PrevRefCounter;// prev catched value T3CC4

static uint32_t RefNextCheck; // end of measure interval point T3CC3
static uint32_t InCatchValue; // current catched value T1CC1


static uint32_t ResultRefCount; // Measure result
static uint32_t ResultInCount;  // Measure result
static uint8_t  ResultReady;    // Measure result ready
static uint8_t  Prescaler = 1;
static uint8_t  MenuCounter;


#define DMA3_TRANSFER_COUNT 6
static uint16_t Dma3CatchArray[DMA3_TRANSFER_COUNT];

#define MeasureInterval 48000 /* Clock in mS */ * 1000 /* mS */

void TIM1_CC_IRQHandler()
{
  if ( TIM1->SR & TIM_SR_CC3IF)
  {
    uint16_t RefHi = RefHiCounter;
    
    TIM1->SR &= ~TIM_SR_CC3IF;
    if ((TIM1->SR & TIM_SR_UIF) && (TIM1->CCR3 < 0x7FFF))
      RefHi++;
      
    if (RefHi == RefNextCheck>>16) // End of the measure
TSetReastart:
    {
      uint16_t Inc = 2;
      uint16_t CurHiIn = InHiCounter;
      uint16_t CurLoIn = TIM3->CNT;
      uint32_t CurInCounter;
      
      if ((TIM3->SR & TIM_SR_UIF) && (CurLoIn < 0x7FFF))
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

      TIM3->CCMR1 = TIM_CCMR1_OC1M_2; // force low
      TIM3->CCR1 = TIM3->CNT - 1; // exclude geting capt in 64000 next tick
      TIM3->SR &= ~TIM_SR_CC1IF; // Clear compare flag
      TIM3->CCMR1 = TIM_CCMR1_OC1M_0; // Set 1 on compare

      CurHiIn = InHiCounter;
      CurLoIn = TIM3->CNT;
      if ((TIM3->SR & TIM_SR_UIF) && (CurLoIn < 0x7FFF))
        CurHiIn++;
      TIM3->CCR1 = CurLoIn + Inc; // generate compare event in some time of input signal
           
      if (Inc == 1 && TIM3->CCR1 - TIM3->CNT - 1 > 0x7FFF) // raise was skipped on Inc == 1
      {
        if (TIM3->SR & TIM_SR_CC1IF) // Very rerary event - edge of input signal in CCR aasigned time
        {
          TIM1->SR &= ~TIM_SR_CC4IF; // This inp raise can be not correct - skip it
          goto TSetReastart;
        }
      }

      CurInCounter = CurLoIn + Inc;
      if ((TIM3->SR & TIM_SR_UIF) && (CurLoIn < 0x7FFF))
        CurHiIn++;
      CurInCounter += ((uint32_t)CurHiIn << 16);
      InCatchValue = CurInCounter;   
      RefNextCheck = RefNextCheck + MeasureInterval;
      TIM1->CCR3 = RefNextCheck;
    } // end next timeout check
  }

  if ( TIM1->SR & TIM_SR_CC4IF)
  {
    uint32_t CurRef = RefHiCounter;
    
    TIM1->SR &= ~TIM_SR_CC4IF;
    if ((TIM1->SR & TIM_SR_UIF) && (TIM1->CCR4 < 0x7FFF))
      CurRef++;
    CurRef = (((uint32_t)CurRef)<<16) + TIM1->CCR4;
    ResultRefCount = CurRef - PrevRefCounter;
    PrevRefCounter = CurRef;
    ResultInCount  = InCatchValue - PrevInCounter;
    PrevInCounter = InCatchValue;
    ResultReady++;
//    TIM1->CCMR1 = TIM_CCMR1_OC1M_2; //Force low level
  }
}

static void SwitchPrescaler(int NeedPrescaler)
{
  uint32_t NewCCMR;
  
  TIM3->CR1 &= ~TIM_CR1_CEN;
  TIM1->CR1 &= ~TIM_CR1_CEN;

  TIM1->CCR3 = MeasureInterval;
  RefNextCheck = 50;
  RefHiCounter = 0;
  TIM1->CNT = 0; // To Start almost immediate
  ResultReady = 0xFF; //skip next display
  
  NewCCMR = TIM1->CCMR1 & ~TIM_CCMR1_IC1PSC;
  Prescaler = 1;
  if (NeedPrescaler)
  {
    NewCCMR |= (TIM_CCMR1_IC1PSC_0|TIM_CCMR1_IC1PSC_1); // prescaler == 8 
    Prescaler = 8;
  }
  TIM1->CCMR1 = NewCCMR;
  
  TIM3->CR1 |= TIM_CR1_CEN;
  TIM1->CR1 |= TIM_CR1_CEN;
}

#define ThresholdValue 16 // 2*8. max Fin = Fref/2. 8 - prescaler CCR2

static void AnalizeFreq()
{
  uint32_t Diff;

  Diff = Dma3CatchArray[DMA3_TRANSFER_COUNT-1] - Dma3CatchArray[1];
  if ((Diff < (DMA3_TRANSFER_COUNT-2)* (ThresholdValue+2)) && // Too higth without prescaler. 16 - for max F without prescaler
       (Prescaler == 1) )
  {
    SwitchPrescaler(1);
  }
  if ((Diff > (DMA3_TRANSFER_COUNT-2)* (ThresholdValue+4)) && // Too low with prescaler.
       (Prescaler >1))
  {
    SwitchPrescaler(0);
  }
}

void TIM1_BRK_UP_TRG_COM_IRQHandler()
{
  if ( TIM1->SR & TIM_SR_UIF)
  {
    RefHiCounter++;
    TIM1->SR &= ~TIM_SR_UIF;
  }

  DMA1_Channel3->CCR &= ~DMA_CCR_EN;

  if (DMA1->ISR & DMA_ISR_TCIF3)
  {
    DMA1->IFCR |= DMA_IFCR_CGIF3;
    AnalizeFreq();
  }
  else // Too low freq < F_MAIN/65000/DMA3_TRANSFER_COUNT
  {
    if (Prescaler > 1 )
      SwitchPrescaler(0);
  }

  DMA1_Channel3->CNDTR = DMA3_TRANSFER_COUNT;
  DMA1_Channel3->CCR |= DMA_CCR_EN;
}

void TIM3_IRQHandler()
{
  InHiCounter++;
  TIM3->SR &= ~TIM_SR_UIF;
}


//void DMA1_Channel2_3_IRQHandler()
//{
//  DMA1->IFCR |= DMA_IFCR_CGIF3;
//}
void TimersInit()
{
  /* TI1 pass throuht CC1 to MMS. CC2 used with prescaler */
  TIM1->CR2 = TIM_CR2_MMS_0|TIM_CR2_MMS_1; //MMC - compare pulse
  TIM1->CCMR1 = TIM_CCMR1_CC1S_1|TIM_CCMR1_CC2S_0| // TI2 source for CC1 and CC2
             TIM_CCMR1_IC2PSC_0|TIM_CCMR1_IC2PSC_1; // prescaler == 8 for CC2

  /* CC4 is used for catch one of input raise throuht T3 */
  /* CC3 for time count - 48000 clocks == 1mS */
  TIM1->CCMR2 = TIM_CCMR2_CC4S_0|TIM_CCMR2_CC4S_1; // TRC source for CC4, CC3  - OUTPUT
  TIM1->ARR = 0xFFFF; 
  TIM1->CCER = TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC4E; // CC1 CC4 capture enable
  TIM1->DIER = TIM_DIER_UIE|TIM_DIER_CC4IE|TIM_DIER_CC3IE|TIM_DIER_CC2DE; // interrupt by update,CC4 capture, CC3 compare, CC2 DMA
  TIM1->CCR3 = MeasureInterval;
  RefNextCheck = MeasureInterval;
  TIM1->SMCR = TIM_SMCR_TS_1|TIM_SMCR_SMS_1|TIM_SMCR_SMS_2; //source is TIM3 MMC,Enable by SMC  - some mode MUST be ON to ON SMC!

  TIM3->ARR = 0xFFFF;
  TIM3->SMCR = 0|TIM_SMCR_SMS_0|TIM_SMCR_SMS_1|TIM_SMCR_SMS_2; // TIM1 MMC is source, external clock 1.
  TIM3->DIER =  TIM_DIER_UIE; // interrupts by update
  TIM3->CCMR1 = TIM_CCMR1_OC1M_2;//Force low level
  TIM3->CR2 = TIM_CR2_MMS_2; // OC1REF as source for MMC
  
//  TIM1->CCER = TIM_CCER_CC1NE; // Out CC1N to PA7
//  TIM1->BDTR = TIM_BDTR_MOE; // All outputs enable
  
  
  NVIC_EnableIRQ(TIM3_IRQn); /* Timer IRQ */
  NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn); /* Timer IRQ */
  NVIC_EnableIRQ(TIM1_CC_IRQn);/* Timer IRQ */
  NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 4);
  TIM3->CR1 |= TIM_CR1_CEN;
  TIM1->CR1 |= TIM_CR1_CEN;
  
#if 1
  TIM14->ARR = 48-1; // test signal 1 MHz
  TIM14->CCR1 = 23;
  TIM14->CCMR1 = TIM_CCMR1_OC1M_1|TIM_CCMR1_OC1M_2; //pwm 1 at PWMOUT (PA7)
  TIM14->CCER = TIM_CCER_CC1E;
  TIM14->CR1 |= TIM_CR1_CEN;
#endif

  DMA1_Channel3->CCR = DMA_CCR_MSIZE_0|DMA_CCR_PSIZE_0|DMA_CCR_MINC/*|DMA_CCR_TCIE */; //16 bit memory and periphery, memory increment,complete interrupt 
  DMA1_Channel3->CNDTR = DMA3_TRANSFER_COUNT;
  DMA1_Channel3->CPAR = (uint32_t)(&TIM1->CCR2);
  DMA1_Channel3->CMAR = (uint32_t)(&Dma3CatchArray[0]);
//  NVIC_EnableIRQ(DMA1_Channel2_3_IRQn);
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


void EventIdle()
{
	__disable_irq();
  if (ResultReady == 1 && EventQueue == 0)
  { 
    ResultReady = 0;
    EventQueue = (KEY_F|KEY_PRESSED_VALUE);
  }
	__enable_irq();
}

void StartFunction(void);
void MainMenu(void);
void PwmSetting(void);

__IO uint16_t* WordAddr;


void WordToChar(uint16_t Word, char* Out)
{
  uint16_t Nibble;
  char* CurOut = Out + 3;
  int i;
  
  for (i=0; i<4;i++)
  {
    Nibble = Word & 0x0F;
    if (Nibble <= 9)
      *CurOut = '0' + Nibble;
    else
      *CurOut = 'A' + Nibble - 10;
    Word = (Word >> 4);
    CurOut--;
  }
}
void ChangeHex()
{
  if ( Event == 0 )
    return;

  if ( Event == EV_FUNC_FIRST )
  {
    MenuCounter = 0;
    LcdClear();
    goto RedrawMenu;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED )
  {
    switch (Event & KEY_MASK)
    {
      case KEY_UP:
        (*WordAddr)++;
        goto RedrawMenu;
      case KEY_DOWN:
        (*WordAddr)--;
        goto RedrawMenu;
      case KEY_ENTER:
        return; /* NONE for short press */
    }
  }
  else if ((Event & (EV_MASK|KEY_MASK)) == (EV_KEY_LONG|KEY_ENTER) )
  {
    CurrentFunc(PwmSetting);
    return;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED ||
       (Event & EV_MASK) == EV_KEY_LONG)
  {
    switch (Event & KEY_MASK)
    {
      case KEY_DOWN|KEY_ENTER:
        *WordAddr = (*WordAddr)>>4;
        goto RedrawMenu;
      case KEY_UP|KEY_ENTER:
        *WordAddr = (*WordAddr)<<4;
        goto RedrawMenu;
    }
  }
  return;
RedrawMenu:
  {
    char HexValue[4];

    WordToChar(*WordAddr, HexValue);
    LcdChr(X_POSITION*2+Y_POSITION*2+ 4 + MUL3, HexValue);
  }
}

void PwmSetting()
{
  if ( Event == 0 )
    return;

  if ( Event == EV_FUNC_FIRST )
  {
    MenuCounter = 0;
    LcdClear();
    goto RedrawMenu;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED )
  {
    switch (Event & KEY_MASK)
    {
      case KEY_UP:
        if ( MenuCounter == 0 )
          MenuCounter = 3;
        else
          MenuCounter = MenuCounter - 1;
        break;
      case KEY_DOWN:
        if ( MenuCounter == 3 )
          MenuCounter = 0;
        else
          MenuCounter = MenuCounter + 1;
        break;
      case KEY_ENTER:
        switch(MenuCounter)
        {
          case 0: //ARR
            WordAddr = (__IO uint16_t*)&TIM14->ARR;
            goto ChangeFunc;
          case 1: //CCR1
            WordAddr = (__IO uint16_t*)&TIM14->CCR1;
            goto ChangeFunc;
          case 2:
            WordAddr = &TIM14->PSC;
            goto ChangeFunc;
          case 3: /* BACK */
            CurrentFunc(MainMenu);
            return;
ChangeFunc:
            CurrentFunc(ChangeHex);
            return;
        }
    }
  }
  else
    return;
RedrawMenu:
  {
    char HexValue[4];

    LcdChr(X_POSITION*0+Y_POSITION*0+ 4 + MUL2, "ARR");
    WordToChar(TIM14->ARR, HexValue);
    LcdChr(X_POSITION*8+Y_POSITION*0+ 4 + MUL2 + (0==MenuCounter)*INVERSE, HexValue);
  
    LcdChr(X_POSITION*0+Y_POSITION*2+ 4 + MUL2, "CCR");
    WordToChar(TIM14->CCR1, HexValue);
    LcdChr(X_POSITION*8+Y_POSITION*2+ 4 + MUL2 + (1==MenuCounter)*INVERSE, HexValue);

    LcdChr(X_POSITION*0+Y_POSITION*4+ 4 + MUL2, "PSC");
    WordToChar(TIM14->PSC, HexValue);
    LcdChr(X_POSITION*8+Y_POSITION*4+ 4 + MUL2 + (2==MenuCounter)*INVERSE, HexValue);
    
    LcdChr(X_POSITION*0+Y_POSITION*6+ 8 + MUL2 + (3==MenuCounter)*INVERSE, "Back");
  }
}

void MainMenu()
{
  if ( Event == 0 )
    return;

  if ( Event == EV_FUNC_FIRST )
  {
    MenuCounter = 0;
    LcdClear();
    goto RedrawMenu;
  }

  if ( (Event & EV_MASK) == EV_KEY_PRESSED )
  {
    switch (Event & KEY_MASK)
    {
      case KEY_UP:
        if ( MenuCounter == 0 )
          MenuCounter = 3;
        else
          MenuCounter = MenuCounter - 1;
        break;
      case KEY_DOWN:
        if ( MenuCounter == 3 )
          MenuCounter = 0;
        else
          MenuCounter = MenuCounter + 1;
        break;
      case KEY_ENTER:
        switch(MenuCounter)
        {
          case 0: //
          case 1:
          case 2:
            break;
          case 3:
            CurrentFunc(PwmSetting);
            return;
        }
    }
  }
  else
    return;
RedrawMenu:
  LcdChr(X_POSITION*0+Y_POSITION*0+ 8 + MUL2 + (0==MenuCounter)*INVERSE, "Menu1");
  LcdChr(X_POSITION*0+Y_POSITION*2+ 8 + MUL2 + (1==MenuCounter)*INVERSE, "Menu2");
  LcdChr(X_POSITION*0+Y_POSITION*4+ 8 + MUL2 + (2==MenuCounter)*INVERSE, "Menu4");
  LcdChr(X_POSITION*0+Y_POSITION*6+ 8 + MUL2 + (3==MenuCounter)*INVERSE, "PWM");
}

void StartFunction()
{
  if (Event == 0)
    return;

  if ( Event == EV_FUNC_FIRST )
  {
    MenuCounter = 0;
    LcdClear();
    return;
  }

  if (Event & EV_KEY_PRESSED)
  {
    switch(Event & KEY_MASK)
    {
      case  KEY_F:
      { 
        volatile double Result;
        uint32_t IntRes;
      
        ResultReady = 0;
        Result = (double)ResultInCount * (double)MAIN_F * (double)1000 / (double)ResultRefCount * Prescaler;
        IntRes = Result/1000000;     
//      if ( IntRes > 0)
        {
          OutValue(0, 0, IntRes, 20, 20);
          LcdChr(14*X_POSITION + Y_POSITION*0 + MUL2 + 1, "k");
        }
        IntRes = ((uint32_t)Result)%1000000;     
        OutValue(3, 0, IntRes, 3, 20);
//      LcdChr(12*X_POSITION + Y_POSITION*3 + MUL2 + 3, "Hz");
        if (Prescaler == 1)
          LcdChr(0*X_POSITION + Y_POSITION*7 + 2, "/1");
        else
          LcdChr(0*X_POSITION + Y_POSITION*7 + 2, "/8");
        break;
      }
      case KEY_ENTER:
      default:
        CurrentFunc(MainMenu);
        return;
    }
  }
}

int main()
{
  RCC->AHBENR |= RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_DMAEN;
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
  EventInit();
  CurrentFunc(StartFunction);

  TimersInit();  

  do
  {
  	EventCheck();
  }
  while(1);
}
