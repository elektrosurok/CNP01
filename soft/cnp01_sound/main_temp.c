#include "stm32l1xx.h"
#include "stm32l1xx_gpio.h"



// my *.h
#include "main.h"
#include "cnp01_init.h"
#include "cnp01_lcd.h"
// my *.c
//#include "cnp01_init.c"
//#include "cnp01_lcd.c"

//===general===
uint8_t tmp=0;
uint8_t action=0;
uint8_t act_tim_state=0;
uint8_t watch_state=0;
int8_t button_cnt[4]={0,0,0,0};
int8_t sleep_mode=0;
//uint8_t no_time=0;

//===clock mode===
uint8_t current_second=0;
uint8_t current_day=0;

//===stopwatch mode===
uint32_t start_time=0;
uint32_t pause_time=0;
//uint32_t split_time=0;
uint32_t chrono_current=0;
uint32_t splits[256];
uint8_t current_split=0;
uint8_t split_view=0;
uint8_t chrono_started=0;
uint8_t chrono_split=0;
uint8_t lap_mode=0;
const uint32_t full_day=22118400;
#ifdef STM32L100
uint16_t chrono_timer=0;
#endif

//===2n time zone===
int8_t time_2_hours=-1;
int8_t time_2_minutes=0;



int main()
{
  init_rcc();
  init_gpio();
  init_lcd();
  init_rtc();
  
  //sound_init(125);
  // Initializing timer
/*    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler=0; //TIM_F/256 - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period=125;
    TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
    
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_Toggle; //переключатель
    TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse=1;
    TIM_OC4Init(TIM4, &TIM_OCInitStruct);
    
    TIM_Cmd(TIM4,ENABLE);
   */
#ifdef DEBUG_MODE
  DBGMCU_Config(DBGMCU_STOP,ENABLE);
#endif    
  
  //RCC_MSIRangeConfig(RCC_MSIRange_2);
  //GPIO_WriteBit(GPIOA, 0, Bit_RESET);
  while (PWR_GetFlagStatus(PWR_FLAG_VOS)!=RESET) ;
  PWR_VoltageScalingConfig(PWR_VoltageScaling_Range2);
  PWR_EnterLowPowerRunMode(ENABLE);
  
 // RCC_ClocksTypeDef clocks;
 // RCC_GetClocksFreq(&clocks);
  //uint32_t tmp;
  //TIM7->CNT=clocks.PCLK1_Frequency;
  
  while(1)
  {
    //RTC_WakeUpCmd(ENABLE);
    if (sleep_mode) PWR_EnterSleepMode(PWR_Regulator_ON,PWR_SLEEPEntry_WFI);
    else /*if (!sleep_mode)*/ PWR_EnterSTOPMode(PWR_Regulator_LowPower,PWR_STOPEntry_WFI);
    //else*/ if (sleep_mode) PWR_EnterSleepMode(PWR_Regulator_ON,PWR_STOPEntry_WFI);
  };
}

// -----------------------------------------------------------------------------
// Interrupts section
// -----------------------------------------------------------------------------


// Interrupts for buttons
void EXTI4_IRQHandler(void)
{
  
  EXTI->IMR &= ~ EXTI_Line4;
  action|=0x02;
  button_cnt[1]=1;
  button_act_A();
  tim11_init(64);
  EXTI_ClearFlag(EXTI_Line4);
}
  
void EXTI9_5_IRQHandler(void)
{
  
  EXTI->IMR &= ~ EXTI_Line5;
  action|=0x01;
  button_cnt[0]=1;
  button_act_A();
  tim11_deinit();sound_deinit();
  EXTI_ClearFlag(EXTI_Line5);
}

void EXTI15_10_IRQHandler(void)
{
  if (EXTI->PR & EXTI_Line11) 
  {
    EXTI->IMR &= ~ EXTI_Line11;
    action|=0x08;
    button_cnt[3]=1;
  }
  if (EXTI->PR & EXTI_Line12) 
  {
    EXTI->IMR &= ~ EXTI_Line12;
    action|=0x04;
    button_cnt[2]=1;
  }
  EXTI_ClearFlag(EXTI_Line11);
  EXTI_ClearFlag(EXTI_Line12);
  button_act_A();
}
// END Interrupts for buttons

// Interrupt for RTC
void RTC_WKUP_IRQHandler(void)
{
  if(RTC_GetITStatus(RTC_IT_WUT) != RESET) {
    // Update time on display, if it's needed
    EXTI_ClearITPendingBit(EXTI_Line20); //OK
    PWR_RTCAccessCmd(ENABLE);
    RTC_ClearITPendingBit(RTC_IT_WUT);
    RTC_ClearFlag(RTC_FLAG_WUTF);
    PWR_RTCAccessCmd(DISABLE);
    //RTC_TimeTypeDef RTC_TimeStruct;
    //RTC_GetTime(RTC_Format_BCD,&RTC_TimeStruct);
    
    //if (!no_time)
    //refresh_time(&RTC_TimeStruct);
    display();
   
    // Update button counters
    if (button_cnt[0]||button_cnt[1]||button_cnt[2]||button_cnt[3])
    for (char i=0;  i<4; i++) 
    {
      
      switch (i)
      {
        case 0: 
          if (button_cnt[i]==-1) 
          {
            action&=0xE;
            button_cnt[i]=0;
            EXTI->IMR |= EXTI_Line5;
          }
          else 
            if (button_cnt[i]&&!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5)) button_cnt[i]=-1;
          break;      
        case 1: 
          if (button_cnt[i]==-1) 
          {
            action&=0xD;
            button_cnt[i]=0;
            EXTI->IMR |= EXTI_Line4;
          }
          else 
            if (button_cnt[i]&&!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4)) button_cnt[i]=-1;
          break;      
        case 2: 
          if (button_cnt[i]==-1) 
          {
            action&=0xB;
            button_cnt[i]=0;
            EXTI->IMR |= EXTI_Line12;
          }
          else 
            if (button_cnt[i]&&!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12)) button_cnt[i]=-1;
          break;      
        case 3: 
          if (button_cnt[i]==-1) 
          {
            action&=0x7;
            button_cnt[i]=0;
            EXTI->IMR |= EXTI_Line11;
          }
          else 
            if (button_cnt[i]&&!GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11)) button_cnt[i]=-1;
          break;      
      }
      if (button_cnt[i]>0) button_cnt[i]++;
      
      if (button_cnt[i]==6) button_act_B();
      if (button_cnt[i]>7) button_act_C();
    }
  }
}

#ifdef STM32L100
void TIM10_IRQHandler(void)
{
  TIM10->SR &= ~TIM_SR_UIF; //—брасываем флаг UIF
  chrono_timer++;
}
#endif

uint8_t temp1=0;

void TIM11_IRQHandler (void)
{
  TIM11->SR &= ~TIM_SR_UIF;
  temp1++;
  if(temp1&1) sound_init(64); else sound_init(128);
  
}




void sound_init (uint32_t period)
{
     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function TIM4 (AF2)
     GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
     GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  
  
  
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler=0; //TIM_F/256 - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period=period;
    TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4,&TIM_TimeBaseInitStruct);
    
    TIM_OCInitTypeDef TIM_OCInitStruct;
    TIM_OCInitStruct.TIM_OCMode=TIM_OCMode_Toggle; //переключатель
    TIM_OCInitStruct.TIM_OCPolarity=TIM_OCPolarity_High;
    TIM_OCInitStruct.TIM_OutputState=TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse=1;
    TIM_OC4Init(TIM4, &TIM_OCInitStruct);
    
    TIM_Cmd(TIM4,ENABLE);
    sleep_mode=1;
}

void sound_deinit (void)
{
  TIM_Cmd(TIM4,DISABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,DISABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // alternate function TIM4 (AF2)
     GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
     GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
  sleep_mode=0;
}

void tim11_init (uint32_t period)
{
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11,ENABLE);
          TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
          TIM_TimeBaseInitStruct.TIM_Prescaler=128 - 1;
          TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
          TIM_TimeBaseInitStruct.TIM_Period=period;
          TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
          TIM_TimeBaseInit(TIM11,&TIM_TimeBaseInitStruct);
          TIM_ETRClockMode2Config(TIM11, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
          TIM11->SR &= ~TIM_SR_UIF;
          TIM_ITConfig(TIM11, TIM_IT_Update, ENABLE);
          NVIC_EnableIRQ(TIM11_IRQn);
          TIM_Cmd(TIM11,ENABLE);
          sleep_mode=1; //##############################################
}

void tim11_deinit (void)
{
  TIM_Cmd(TIM11,DISABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11,DISABLE);
}

//=============================================================================
// Logic
//=============================================================================

// Button functions
void button_act_A()
{
  switch (watch_state)
  {
  case 0x00:
    switch(action)
    {
    case 0x01:
      new_watch_state(50);
      
      break;
    case 0x02:
      new_watch_state(16);
      break;
    
    }
  break;
  case 0x01:
    switch (action)
    {
    case 0x01: //State 0x01
      RTC_GetTime(RTC_Format_BCD, &RTC_TimeStruct);
      RTC_TimeStruct.RTC_Seconds=0;
      RTC_TimeStruct.RTC_Minutes=IncreaseBcd(RTC_TimeStruct.RTC_Minutes);
      if (RTC_TimeStruct.RTC_Minutes==0x60) 
      {
        RTC_TimeStruct.RTC_Minutes=0;
        RTC_TimeStruct.RTC_Hours=IncreaseBcd(RTC_TimeStruct.RTC_Hours);
        if (RTC_TimeStruct.RTC_Hours==0x24) RTC_TimeStruct.RTC_Hours=0;
      }
      //   tmp++;
      init_RTC_Time(RTC_Format_BCD, &RTC_TimeStruct); //???
      //refresh_time(&RTC_TimeStruct);                  //???
      break;
    case 0x02:
      RTC_GetTime(RTC_Format_BCD, &RTC_TimeStruct);
      RTC_TimeStruct.RTC_Seconds=0;
      init_RTC_Time(RTC_Format_BCD, &RTC_TimeStruct);
      //refresh_time(&RTC_TimeStruct);
      break;
    case 0x04:
      new_watch_state(0x02);
      break;
    }
    break;
  case 0x02: //State 0x02
    switch (action)
    {
    case 0x01:
      RTC_GetTime(RTC_Format_BCD, &RTC_TimeStruct);
      RTC_TimeStruct.RTC_Minutes=IncreaseBcd(RTC_TimeStruct.RTC_Minutes);
      if (RTC_TimeStruct.RTC_Minutes==0x60) 
      {
        RTC_TimeStruct.RTC_Minutes=0;
      }
      init_RTC_Time(RTC_Format_BCD, &RTC_TimeStruct);
      current_second=0xFF;
      //refresh_time(&RTC_TimeStruct);
      break;
    case 0x02:
      RTC_GetTime(RTC_Format_BCD, &RTC_TimeStruct);
      RTC_TimeStruct.RTC_Minutes=DecreaseBcd(RTC_TimeStruct.RTC_Minutes);
      if (RTC_TimeStruct.RTC_Minutes==0x99) 
      {
        RTC_TimeStruct.RTC_Minutes=0x59;
      }
      init_RTC_Time(RTC_Format_BCD, &RTC_TimeStruct);
      current_second=0xFF;
      //refresh_time(&RTC_TimeStruct);
      break;
    case 0x04:
      new_watch_state(0x03);
      break;    
    }
    break;
  case 0x03: //State 0x03
    switch (action)
    {
    case 0x01:
      RTC_GetTime(RTC_Format_BCD, &RTC_TimeStruct);
      RTC_TimeStruct.RTC_Hours=IncreaseBcd(RTC_TimeStruct.RTC_Hours);
      if (RTC_TimeStruct.RTC_Hours==0x24) 
      {
        RTC_TimeStruct.RTC_Hours=0;
      }
      init_RTC_Time(RTC_Format_BCD, &RTC_TimeStruct);
      current_second=0xFF;
      //refresh_time(&RTC_TimeStruct);
      break;
    case 0x02:
      RTC_GetTime(RTC_Format_BCD, &RTC_TimeStruct);
      RTC_TimeStruct.RTC_Hours=DecreaseBcd(RTC_TimeStruct.RTC_Hours);
      if (RTC_TimeStruct.RTC_Hours==0x99) 
      {
        RTC_TimeStruct.RTC_Hours=0x23;
      }
      init_RTC_Time(RTC_Format_BCD, &RTC_TimeStruct);
      current_second=0xFF;
      //refresh_time(&RTC_TimeStruct);
      break;
    case 0x04:
      new_watch_state(0x04);
      break;    
    }
    break;
    
  case 0x04: //State 0x04
    switch (action)
    {
    case 0x01:
      RTC_GetDate(RTC_Format_BCD, &RTC_DateStruct);
      RTC_DateStruct.RTC_Year=IncreaseBcd(RTC_DateStruct.RTC_Year);
      if (RTC_DateStruct.RTC_Year==0x70) 
      {
        RTC_DateStruct.RTC_Year=0;
      }
      init_RTC_Date(RTC_Format_BCD, &RTC_DateStruct);
      break;
    case 0x02:
      RTC_GetDate(RTC_Format_BCD, &RTC_DateStruct);
      RTC_DateStruct.RTC_Year=DecreaseBcd(RTC_DateStruct.RTC_Year);
      if (RTC_DateStruct.RTC_Year==0x99) 
      {
        RTC_DateStruct.RTC_Year=0x69;
      }
      init_RTC_Date(RTC_Format_BCD, &RTC_DateStruct);
      break;
    case 0x04:
      new_watch_state(0x05);
      break;    
    }
    break;
    
  case 0x05: //State 0x05
    switch (action)
    {
    case 0x01:
      RTC_GetDate(RTC_Format_BCD, &RTC_DateStruct);
      RTC_DateStruct.RTC_Month=IncreaseBcd(RTC_DateStruct.RTC_Month);
      if (RTC_DateStruct.RTC_Month==0x13) RTC_DateStruct.RTC_Month=1;
      init_RTC_Date(RTC_Format_BCD, &RTC_DateStruct);
      break;
    case 0x02:
      RTC_GetDate(RTC_Format_BCD, &RTC_DateStruct);
      RTC_DateStruct.RTC_Month=DecreaseBcd(RTC_DateStruct.RTC_Month);
      if (RTC_DateStruct.RTC_Month==0) RTC_DateStruct.RTC_Month=0x12;
      init_RTC_Date(RTC_Format_BCD, &RTC_DateStruct);
      break;
    case 0x04:
      new_watch_state(0x06);
      break;    
    }
    break;
    
  case 0x06: //State 0x06
        
    switch (action)
    {
    case 0x01:
      RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
      RTC_DateStruct.RTC_Date++;
     
      if (RTC_DateStruct.RTC_Date>31) 
      {
        RTC_DateStruct.RTC_Date=1;
      }
      if ((RTC_DateStruct.RTC_Date>30)&&((RTC_DateStruct.RTC_Month==4)||(RTC_DateStruct.RTC_Month==6)||(RTC_DateStruct.RTC_Month==9)||(RTC_DateStruct.RTC_Month==11))) 
      {
        RTC_DateStruct.RTC_Date=1;
      }
      if ((RTC_DateStruct.RTC_Date>29)&&(RTC_DateStruct.RTC_Month==2)&&(!(RTC_DateStruct.RTC_Year%4))) 
      {
        RTC_DateStruct.RTC_Date=1;
      }
      if ((RTC_DateStruct.RTC_Date>28)&&(RTC_DateStruct.RTC_Month==2)&&(RTC_DateStruct.RTC_Year%4)) 
      {
        RTC_DateStruct.RTC_Date=1;
      }
      init_RTC_Date(RTC_Format_BIN, &RTC_DateStruct);
      break;
    case 0x02:
      RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
      RTC_DateStruct.RTC_Date--;
      
      if (RTC_DateStruct.RTC_Date==0) 
      {
        RTC_DateStruct.RTC_Date=31;
        if ((RTC_DateStruct.RTC_Month==4)||(RTC_DateStruct.RTC_Month==6)||(RTC_DateStruct.RTC_Month==9)||(RTC_DateStruct.RTC_Month==11)) RTC_DateStruct.RTC_Date=30;
        if ((RTC_DateStruct.RTC_Month==2)&&(!(RTC_DateStruct.RTC_Year%4))) RTC_DateStruct.RTC_Date=29;
        if ((RTC_DateStruct.RTC_Month==2)&&(RTC_DateStruct.RTC_Year%4)) RTC_DateStruct.RTC_Date=28;
      }
      init_RTC_Date(RTC_Format_BIN, &RTC_DateStruct);
      break;
    case 0x04:
      RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
      RTC_DateStruct.RTC_WeekDay = week_day(RTC_DateStruct.RTC_Date, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Year+2000);
      init_RTC_Date(RTC_Format_BIN, &RTC_DateStruct);
      new_watch_state(0x01);
      break;    
    }
    break;
  //---------------------------------------------------------------------------
  case 16:      //2nd time zone
    switch(action)
    {
    case 0x01:
      break;
    case 0x02:
      new_watch_state(0);
      break;
    case 0x04:
      break;
    }
    break;
  case 17:     
    switch(action)
    {
    case 0x01:
      if (time_2_hours<23) time_2_hours++;
      break;
    case 0x02:
      if (time_2_hours>-23) time_2_hours--;
      break;
    case 0x04:
      new_watch_state(18);
      break;
    }
    break;
  case 18:  
    switch(action)
    {
    case 0x01:
      if (time_2_minutes<45) time_2_minutes=time_2_minutes+15;
      break;
    case 0x02:
      if (time_2_minutes>-45) time_2_minutes=time_2_minutes-15;
      break;
    case 0x04:
      new_watch_state(17);
      break;
    }
    break;
    
  //----------------------------------------------------------------------------   
  case 50:      //Stopwatch
    switch(action)
    {
    case 0x01:
      new_watch_state(51);
      split_view=1;
      break;
    case 0x02:
      chrono_update();
      if (chrono_started==1) 
      {
        //split_time=chrono_current; 
        chrono_split=1;
        current_split++;
        splits[current_split]=chrono_current;
        //if (lap_mode&&current_split) split_time=difference(split_time,splits[current_split-1]);
      }
      else
      {
        chrono_split=0;
        lap_mode^=1;
      }
      break;
    case 0x04:
      if (chrono_started==0) 
      {
        if (pause_time==0) 
        {
          current_split=0;
#ifdef STM32L100
          //disable stop but enable sleep
          sleep_mode=1;
          //start TIM10
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,ENABLE);
    
          TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
          TIM_TimeBaseInitStruct.TIM_Prescaler=128 - 1;
          TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
          TIM_TimeBaseInitStruct.TIM_Period=0xFFFF;
          TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
          TIM_TimeBaseInit(TIM10,&TIM_TimeBaseInitStruct);
          TIM_ETRClockMode2Config(TIM10, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
          TIM10->SR &= ~TIM_SR_UIF;
          TIM_ITConfig(TIM10, TIM_IT_Update, ENABLE);
          //TIM_ClearFlag(TIM10, TIM_FLAG_Update);
          NVIC_EnableIRQ(TIM10_IRQn);
          TIM_Cmd(TIM10,ENABLE);
#endif
        }
        start_time=timestamp();
        //if (start_time<pause_time) start_time+=full_day;
        start_time=difference(start_time,pause_time);
        chrono_started=1;
        //lcd_Blink(0x1C00);
      }
      else 
      {
        chrono_update();
        pause_time=chrono_current;
        current_split++;
        splits[current_split]=chrono_current;
        
        chrono_started=0;
        //lcd_Blink(0);
      }
      break;
    }
    break;
  
  case 51:
    switch(action)
    {
    case 0x01:
      new_watch_state(0);
      break; 
    
    case 0x02:
      split_view++;
      if(split_view>current_split) split_view=1; //################################
      break;
    case 0x04:
      lap_mode^=1;
      break;
    }
    break;
 //----------------------------------------------------------------------------
  }
}


void button_act_B()
{
  switch (watch_state)
  {
  case 0x00: // State
    switch (action)
    {
    case 0x04:
      new_watch_state(0x01);
      break;
    }
    break;
  case 0x01: // State
    switch (action)
    {
    case 0x04:
      new_watch_state(0x00);
      break;
    }
    break;
  case 0x02: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    case 0x04:
      new_watch_state(0x00);
      break;
    }
    break;
  case 0x03: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    case 0x04:
      new_watch_state(0x00);
      break;
    }
  case 0x04: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    case 0x04:
      new_watch_state(0x00);
      break;
    }
    break;
  case 0x05: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    case 0x04:
      new_watch_state(0x00);
      break;
    }
    break;
  case 0x06: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    case 0x04:
      new_watch_state(0x00);
      break;
    }
    break;
    
  //---------------------------------------------------------------------------
  case 16:      //2nd time zone
    switch(action)
    {
    case 0x01:
      break;
    case 0x02:
      break;
    case 0x04:
      new_watch_state(17);
      break;
    }
    break;
  case 17:      //2nd time zone
    switch(action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    case 0x04:
      new_watch_state(16);
      break;
    }
    break;
  case 18:      //2nd time zone
    switch(action)
    {
    case 0x01:
      break;
    case 0x02:
      break;
    case 0x04:
      new_watch_state(16);
      break;
    }
    break;
  //---------------------------------------------------------------------------
  case 50:      //Stopwatch
    switch(action)
    {
    case 0x04:
      chrono_started=0;
      pause_time=0;
      chrono_split=0;
      chrono_current=0;
      splits[0]=0;
      //current_split=0;
      //lcd_Blink(0);
#ifdef STM32L100
      //enable stop but disable sleep
      sleep_mode=0;
      //start TIM10
      TIM_Cmd(TIM10,DISABLE);
      NVIC_DisableIRQ(TIM10_IRQn);
      TIM_ITConfig(TIM10, TIM_IT_Update, DISABLE);
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10,DISABLE);
#endif
      break;
    }
    break;
 
  case 51:
    switch(action)
    {
    case 0x02:
      button_act_A();
      break;
    }
    break;
  } 
}

void button_act_C()
{
  switch (watch_state)
  {
  case 0x02: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    }
    break;
  case 0x03: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    }
    break;
  case 0x04: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    }
    break;
  case 0x05: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    }
    break;
  case 0x06: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    }
    break;
  case 17: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    }
    break;
  case 18: // State
    switch (action)
    {
    case 0x01:
      button_act_A();
      break;
    case 0x02:
      button_act_A();
      break;
    }
    break;
    
  case 51:
    switch(action)
    {
    case 0x02:
      button_act_A();
      break;
    }
    break;
    
  
  }
}

void new_watch_state (uint8_t state)
{
  watch_state=state;
  switch (state)
  {
  case 0x00:
    lcd_Blink(0);
    current_day=0;
    current_second=0xFF;
    break;
  case 0x01:          
    lcd_Blink(0x0003);
    current_day=0;
    current_second=0xFF;
#ifndef DEBUG_MODE
    DBGMCU_Config(DBGMCU_STOP,ENABLE);
#endif
    break;
  case 0x02:
    lcd_Blink(0x000C);
#ifndef DEBUG_MODE
    DBGMCU_Config(DBGMCU_STOP,DISABLE);
#endif
    break;
  case 0x03:
    lcd_Blink(0x0030);
    break;
  case 0x04:
    
    lcd_Blink(0x000C);;
    break;
  case 0x05:
    lcd_Blink(0x00C0);
    break;
  case 0x06:
    lcd_Blink(0x0300);
    break;
  case 0x07:
    lcd_Blink(0x1C00);
    break;
  case 16:
    lcd_Blink(0x00);
    break;
  case 17:
    lcd_Blink(0x0030);
    current_day=0;
    break;
  case 18:
    lcd_Blink(0x000C);
    current_day=0;
    break;
  case 50:
    lcd_Blink(0x00);
    current_day=0; // enable update of date when returns to 0-state
    break;
  }
}



//LCD output
inline static void display(void)
{
  

     switch(watch_state)
     {
     case 0:
     case 1:
     case 2:
     case 3:
       
     RTC_GetTime(RTC_Format_BCD,&RTC_TimeStruct);  
     if (RTC_TimeStruct.RTC_Seconds!=current_second)
     {
       current_second = RTC_TimeStruct.RTC_Seconds;
       refresh_time(&RTC_TimeStruct);
       
       RTC_GetDate(RTC_Format_BCD,&RTC_DateStruct);
       if (RTC_DateStruct.RTC_Date!=current_day)
       {
         current_day = RTC_DateStruct.RTC_Date;
         refresh_date(&RTC_DateStruct);
         lcd_SetAllDashes((1<<7)+(1<<10));
       }
       //lcd_SetAllDashes((1<<7)+(1<<10));
       /*uint32_t i=RTC_GetSubSecond();
       i=(i&0xFF)^0xFF;
       i=((i<<2)+(i<<5)+(i<<6))>>8;
      
       uint8_t j=ByteToBcd2(i&0xFF);
       lcd_SetChar(7,font[j&0x0F]);
       lcd_SetChar(6,font[j>>4]);  */
     }
     
     break;
     
     case 4:
     case 5:
     case 6:
       
       RTC_GetDate(RTC_Format_BCD,&RTC_DateStruct);
       //RTC_GetTime(RTC_Format_BCD,&RTC_TimeStruct);
       refresh_date(&RTC_DateStruct);
       lcd_SetChar(8,font[2]);
       lcd_SetChar(9,font[0]);
       lcd_SetChar(11,font[RTC_DateStruct.RTC_Year&0x0F]);
       lcd_SetChar(10,font[(RTC_DateStruct.RTC_Year&0xF0)>>4]);
       lcd_SetChar(12,0);
       lcd_SetChar(13,0);
       lcd_SetDash(7,1);
        lcd_SetAllDashes((1<<7));
       break;
       
     case 16:
     case 17:
     case 18:
       RTC_GetTime(RTC_Format_BCD,&RTC_TimeStruct);  
       if (RTC_TimeStruct.RTC_Seconds!=current_second)
       {
         current_second = RTC_TimeStruct.RTC_Seconds;
         int8_t temp_minutes=0;
         int8_t temp_hours=0;
         int8_t temp_date=0;
         if (time_2_minutes)
         {
           temp_minutes=(int8_t)Bcd2ToByte(RTC_TimeStruct.RTC_Minutes);
           temp_minutes=temp_minutes+time_2_minutes;
           if (temp_minutes>60)
           {
             temp_minutes=temp_minutes-60;
             temp_hours=temp_hours+1;
           }
           if (temp_minutes<0)
           {
             temp_minutes=temp_minutes+60;
             temp_hours=temp_hours-1;
           }
           RTC_TimeStruct.RTC_Minutes=ByteToBcd2((uint8_t)temp_minutes);
         }
         if (time_2_hours)
         {
           temp_hours=temp_hours+time_2_hours+(int8_t)Bcd2ToByte(RTC_TimeStruct.RTC_Hours);
           //temp_hours=temp_hours+time_2_hours;
           if (temp_hours>24)
           {
             temp_hours=temp_hours-24;
             temp_date=1;
           }
           if (temp_hours<0)
           {
             temp_hours=temp_hours+24;
             temp_date=-1;
           }
           RTC_TimeStruct.RTC_Hours=ByteToBcd2((uint8_t)temp_hours);
         }
         refresh_time(&RTC_TimeStruct);
         
         RTC_GetDate(RTC_Format_BCD,&RTC_DateStruct);
         if (RTC_DateStruct.RTC_Date!=current_day)
         {
           int8_t temp_day=0;
           int8_t temp_month=0;
           uint8_t temp_year=0;
           current_day = RTC_DateStruct.RTC_Date;
           if (temp_date)
           {
             temp_day=RTC_DateStruct.RTC_WeekDay+temp_date;
             if (temp_day<1) temp_day=temp_day+7;
             if (temp_day>7) temp_day=temp_day-7;
             RTC_DateStruct.RTC_WeekDay=temp_day;
             
             temp_date=temp_date+(int8_t)Bcd2ToByte(RTC_DateStruct.RTC_Date);
             temp_month=Bcd2ToByte(RTC_DateStruct.RTC_Month);
             if (temp_date>28)
             {
               uint8_t temp_year=Bcd2ToByte(RTC_DateStruct.RTC_Year)%4;
               if (((temp_year)&&(temp_month==2))||((temp_date>29)&&(temp_month==2))||((temp_date>30)&&((temp_month==4)||(temp_month==6)||(temp_month==9)||(temp_month==11)))||(temp_date>31)) 
               {
                 temp_month=temp_month+1;
                 if (temp_month>12) temp_month=1;
                 RTC_DateStruct.RTC_Month=ByteToBcd2(temp_month);
                 temp_date=1;
               }
             }
             if (temp_date<1) 
             {
               temp_month=temp_month-1;
               if (temp_month<1) temp_month=12;
               switch (temp_month)
               {
               case 1:
               case 2:
               case 4:
               case 6:
               case 8:
               case 9:
               case 11: 
                 temp_date=31;
                 break;
               case 5: 
               case 7: 
               case 10: 
               case 12:
                 temp_date=31;
                 break;
               case 3: 
                 temp_year=Bcd2ToByte(RTC_DateStruct.RTC_Year);
                 if (temp_year%4) temp_date=28;
                 else temp_date=29;
                 break;
               }
               RTC_DateStruct.RTC_Month=ByteToBcd2(temp_month);
             }
             
             RTC_DateStruct.RTC_Date=ByteToBcd2(temp_date);
             
           }
           refresh_date(&RTC_DateStruct);
           lcd_SetAllDashes((1<<7)+(1<<10));
         }
       }
       break;
     
     case 50:
       lcd_SetAllDashes(0);
       if(chrono_started==1) 
       {chrono_update();
        chrono_bcd(chrono_current);
       }
       else chrono_bcd(pause_time);
       
       if (chrono_split==1) //split_bcd(split_time); 
       {
         if ((!lap_mode))
         split_bcd(splits[current_split]);
         else
         split_bcd(difference(splits[current_split],splits[current_split-1]));
         
       }
       else
       {
       lcd_SetChar(3,0);
       lcd_SetChar(4,0);
       lcd_SetChar(6,0);
       lcd_SetChar(7,0);
       lcd_SetWord(9);
       if(!lap_mode) lcd_SetChar(5,font[5]);
       else lcd_SetChar(5,0x38);
       }
       break;
       
     case 51:
       lcd_SetAllDashes(0);
       lcd_SetChar(8,0);
       lcd_SetChar(10,0);
       if ((!lap_mode))
       split_bcd(splits[split_view]);
       else
       split_bcd(difference(splits[split_view],splits[split_view-1]));
       
       
       lcd_SetChar(11,font[split_view/100]);
       uint8_t split_view_bcd=ByteToBcd2(split_view%100);
       lcd_SetChar(12,font[split_view_bcd>>4]);
       lcd_SetChar(13,font[split_view_bcd&0x0F]);
       
       if(!lap_mode) lcd_SetChar(9,font[5]);
       else lcd_SetChar(9,0x38);
       break; 
     }
     
     
     
     if ((!LCD_blinkmask) || (button_cnt[0]>7) || (button_cnt[1]>7))
       lcd_no_blink();
     else 
       lcd_blinker();
}

void chrono_update(void)
{
  uint32_t temp_time;
  
  
  temp_time=timestamp();
  //if (temp_time<start_time) temp_time+=full_day;
  temp_time=difference(temp_time,start_time);
  chrono_current=temp_time;
  
  
  
}

void chrono_bcd (uint32_t temp_time)
{
  uint8_t chrono_time[4]={0,0,0,0};
  uint8_t i=temp_time&0xFF;
  chrono_time[0]=ByteToBcd2(((i<<2)+(i<<5)+(i<<6))>>8);
  temp_time=temp_time>>8;
  chrono_time[1]=ByteToBcd2(temp_time%60);
  temp_time/=60;
  chrono_time[2]=ByteToBcd2(temp_time%60);
  chrono_time[3]=ByteToBcd2(temp_time/60);
  if ((chrono_time[2]>=0x30)||(chrono_time[3]>0))
  {
  lcd_SetChar(8,font[chrono_time[3]>>4]);
  lcd_SetChar(9,font[chrono_time[3]&0x0F]);
  lcd_SetChar(10,font[chrono_time[2]>>4]);
  lcd_SetChar(11,font[chrono_time[2]&0x0F]);
  lcd_SetChar(12,font[chrono_time[1]>>4]);
  lcd_SetChar(13,font[chrono_time[1]&0x0F]);
  lcd_SetDash(10,1);
  lcd_SetDash(13,0);
  lcd_SetDash(14,0);
  
  }
  else
  {
  lcd_SetChar(8,font[chrono_time[2]>>4]);
  lcd_SetChar(9,font[chrono_time[2]&0x0F]);
  lcd_SetChar(10,font[chrono_time[1]>>4]);
  lcd_SetChar(11,font[chrono_time[1]&0x0F]);
  lcd_SetChar(12,font[chrono_time[0]>>4]);
  lcd_SetChar(13,font[chrono_time[0]&0x0F]);
  lcd_SetDash(10,0);
  lcd_SetDash(13,1);
  lcd_SetDash(14,1);
  }
}
void split_bcd (uint32_t temp_time)
{
  uint8_t chrono_time[4]={0,0,0,0};
  uint8_t i=temp_time&0xFF;
  chrono_time[0]=ByteToBcd2(((i<<2)+(i<<5)+(i<<6))>>8);
  temp_time=temp_time>>8;
  chrono_time[1]=ByteToBcd2(temp_time%60);
  temp_time/=60;
  chrono_time[2]=ByteToBcd2(temp_time%60);
  chrono_time[3]=ByteToBcd2(temp_time/60);
  if ((chrono_time[3]>0))
  {
  lcd_SetChar(1,font[chrono_time[3]>>4]);
  lcd_SetChar(2,font[chrono_time[3]&0x0F]);
  lcd_SetChar(4,font[chrono_time[2]>>4]);
  lcd_SetChar(5,font[chrono_time[2]&0x0F]);
  lcd_SetChar(6,font[chrono_time[1]>>4]);
  lcd_SetChar(7,font[chrono_time[1]&0x0F]);
  lcd_SetDash(8,1);
  lcd_SetDash(9,1);
  lcd_SetDash(11,0);
  lcd_SetDash(12,0);
  }
  else
  {
  lcd_SetChar(1,font[chrono_time[2]>>4]);
  lcd_SetChar(2,font[chrono_time[2]&0x0F]);
  lcd_SetChar(4,font[chrono_time[1]>>4]);
  lcd_SetChar(5,font[chrono_time[1]&0x0F]);
  lcd_SetChar(6,font[chrono_time[0]>>4]);
  lcd_SetChar(7,font[chrono_time[0]&0x0F]);
  lcd_SetDash(8,0);
  lcd_SetDash(9,0);
  lcd_SetDash(11,1);
  lcd_SetDash(12,1);
  }
}

uint32_t timestamp(void)
{
   RTC_GetTime(RTC_Format_BIN, &RTC_TimeStruct);
#ifdef STM32L100
   return ((chrono_timer<<16)+TIM_GetCounter(TIM10));
#endif
   return (((uint32_t)RTC_TimeStruct.RTC_Seconds+60*(uint32_t)RTC_TimeStruct.RTC_Minutes+60*60*(uint32_t)RTC_TimeStruct.RTC_Hours)<<8)+(RTC_GetSubSecond()^0xFF);
}

uint32_t difference(uint32_t timestamp1, uint32_t timestamp2)
{
  if (timestamp1<timestamp2) timestamp1+=full_day;
  return timestamp1-timestamp2;
}

// Main mode functions

void refresh_time(RTC_TimeTypeDef *RTC_TimeStruct)
{
  lcd_SetChar(13,font[RTC_TimeStruct->RTC_Seconds&0x0F]);
  lcd_SetChar(12,font[(RTC_TimeStruct->RTC_Seconds&0xF0)>>4]);
  lcd_SetChar(11,font[RTC_TimeStruct->RTC_Minutes&0x0F]);
  lcd_SetChar(10,font[(RTC_TimeStruct->RTC_Minutes&0xF0)>>4]);
  lcd_SetChar(9, font[RTC_TimeStruct->RTC_Hours&0x0F]);
  lcd_SetChar(8, font[(RTC_TimeStruct->RTC_Hours&0xF0)>>4]);
  
}

void refresh_date(RTC_DateTypeDef *RTC_DateStruct)
{
  //RTC_DateTypeDef         RTC_DateStruct;
  //RTC_GetDate(RTC_Format_BCD,&RTC_DateStruct);
  lcd_SetChar(7,font[RTC_DateStruct->RTC_Month&0x0F]);
  lcd_SetChar(6,font[(RTC_DateStruct->RTC_Month&0xF0)>>4]);
  lcd_SetChar(5,font[RTC_DateStruct->RTC_Date&0x0F]);
  lcd_SetChar(4,font[(RTC_DateStruct->RTC_Date&0xF0)>>4]);
  lcd_SetWord(RTC_DateStruct->RTC_WeekDay);
}


/**
  * @brief  Converts a 2 digit decimal to BCD format.
  * @param  Value: Byte to be converted.
  * @retval Converted byte
  */
uint8_t ByteToBcd2(uint8_t Value)
{
  uint8_t bcdhigh = 0;
  
  while (Value >= 10)
  {
    bcdhigh++;
    Value -= 10;
  }
  
  return  ((uint8_t)(bcdhigh << 4) | Value);
}

/**
  * @brief  Convert from 2 digit BCD to Binary.
  * @param  Value: BCD value to be converted.
  * @retval Converted word
  */
uint8_t Bcd2ToByte(uint8_t Value)
{
  uint8_t tmp = 0;
  tmp = ((uint8_t)(Value & (uint8_t)0xF0) >> (uint8_t)0x4) * 10;
  return (tmp + (Value & (uint8_t)0x0F));
}

//Increases a BCD value by 1.
uint8_t IncreaseBcd (uint8_t Value)
{
  Value++;
  if ((Value&(uint8_t)0x0F) > (uint8_t)0x09) Value += (uint8_t)0x06;
  if (Value > (uint8_t)0xA0) Value=(uint8_t)0;
  return Value;
}

//Decreases a BCD value by 1.
uint8_t DecreaseBcd (uint8_t Value)
{
  Value--;
  if ((Value&(uint8_t)0x0F) > (uint8_t)0x09) Value -= (uint8_t)0x06;
   if (Value > (uint8_t)0xA0) Value=(uint8_t)0x99;
  return Value;
}


// Calculates week day
uint8_t week_day(uint8_t day, uint8_t month, uint32_t year)
{
    if (month < 3)
    {
      month=month+10;
      year--;
    }
    else
      month=month-2;
      uint8_t century = year / 100;
      uint8_t yr = year % 100;
      int8_t dw = (((26 * month - 2) / 10) + day + yr + (yr / 4) + (century / 4) - (2 * century)) % 7;
    if (dw <= 0) return (dw + 7);
    else return dw;
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %drn", file, line) */
 
  /* Infinite loop */
  while (1)
  {
  }
}
#endif