#include "cnp01_init.h"
#include "cnp01_lcd.h"
#include "stm32l1xx_adc.h"
#include "main.h"


// Init structures
GPIO_InitTypeDef        GPIO_InitStructure;
RTC_InitTypeDef         RTC_InitStructure; 
RTC_TimeTypeDef         RTC_TimeStruct;
RTC_DateTypeDef         RTC_DateStruct;
NVIC_InitTypeDef        NVIC_InitStructure; 
EXTI_InitTypeDef        EXTI_InitStructure;
LCD_InitTypeDef         LCD_InitStructure; 

ErrorStatus init_RTC_Time(uint32_t RTC_Format, RTC_TimeTypeDef* RTC_TimeStruct)
{
  PWR_RTCAccessCmd(ENABLE);
  //RTC_WriteProtectionCmd(DISABLE);
  //RTC_EnterInitMode();
  ErrorStatus e=RTC_SetTime(RTC_Format, RTC_TimeStruct);
  //RTC_ExitInitMode();
  //RTC_WriteProtectionCmd(ENABLE);
  return e;
}

ErrorStatus init_RTC_Date(uint32_t RTC_Format, RTC_DateTypeDef* RTC_DateStruct)
{
  PWR_RTCAccessCmd(ENABLE);
  //RTC_WriteProtectionCmd(DISABLE);
  //RTC_EnterInitMode();
  ErrorStatus e=RTC_SetDate(RTC_Format, RTC_DateStruct);
  //RTC_ExitInitMode();
  //RTC_WriteProtectionCmd(ENABLE);
  return e;
}

void init_rcc(void)
{
  
  //Enable the Power Controller (PWR) APB1 interface clock
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR,ENABLE);
  // Access for RTC registers
  PWR_RTCAccessCmd(ENABLE);
  
  /* Enable Low Speed External clock */
  RCC_LSEConfig(RCC_LSE_ON); 

  /* Wait till LSE is ready */
  while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);

  /* Select LSE clock as RCC Clock source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_LCD, ENABLE);
  
  /*RCC_RTCResetCmd(ENABLE);
  RCC_RTCResetCmd(DISABLE);*/
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
  //RCC_RTCCLKCmd	(ENABLE); //???
  
  PWR_FastWakeUpCmd(ENABLE);
}

void init_rtc(void)
{
  /* Enable the PWR clock */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
 
  /* Allow access to RTC */
  PWR_RTCAccessCmd(ENABLE);
 
  /* Enable the LSE OSC */
  RCC_LSEConfig(RCC_LSE_ON);
   
  /* Wait till LSE is ready */ 
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET) {  }
 
  /* Select the RTC Clock Source */
  RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
    
  /* Enable the RTC Clock */
  RCC_RTCCLKCmd(ENABLE);
 
  /* Wait for RTC APB registers synchronisation */
  RTC_WaitForSynchro();
 
  /* Calendar Configuration */
  RTC_InitStructure.RTC_AsynchPrediv = 0x7F;
  RTC_InitStructure.RTC_SynchPrediv = 0xFF;
  RTC_InitStructure.RTC_HourFormat = RTC_HourFormat_24;
  RTC_Init(&RTC_InitStructure); 
 
  /*
39.  To enable the RTC Wakeup interrupt, the following sequence is required:
40.  1. Configure and enable the EXTI Line 20 in interrupt mode and select the rising edge
41.  sensitivity.
42.  2. Configure and enable the RTC_WKUP IRQ channel in the NVIC.
43.  3. Configure the RTC to generate the RTC wakeup timer event.
44.   
45.  System reset, as well as low power modes (Sleep, Stop and Standby) have no influence on
46.  the wakeup timer.
47.  */
  /* EXTI configuration *******************************************************/
  EXTI_ClearITPendingBit(EXTI_Line20);
  EXTI_InitStructure.EXTI_Line = EXTI_Line20;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
   
  /* Enable the RTC Wakeup Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
   
  RTC_WakeUpCmd(DISABLE);
  PWR_RTCAccessCmd(ENABLE);
  /* 3.1.3 from AN3371 using hardware RTC
66.  RTC->CR
67.  WUCKSEL -> (only when RTC->CR WUTE = 0; RTC->ISR WUTWF = 1)
68.  000: RTC/16 clock is selected
69.  001: RTC/8 clock is selected
70.  010: RTC/4 clock is selected
71.  011: RTC/2 clock is selected
72.  10x: ck_spre (usually 1 Hz) clock is selected
73.  11x: ck_spre (usually 1 Hz) clock is selected and 216 is added to the WUT counter value
74.  from 1s to 18 hours when WUCKSEL [2:1] = 10
75.  WUTE - Enable Control Bit
76.  WUTIE - Interrupt enable bit
77.  RTC->ISR
78.  WUTF - efent flag
79.  */
  RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div2);
  RTC_SetWakeUpCounter(0xFFF);
  //The WUTF flag must then be cleared by software.
  RTC_ClearITPendingBit(RTC_IT_WUT); //ClearITPendingBit clears also the flag
  RTC_ClearFlag(RTC_FLAG_WUTF); //MANDATORY!
  RTC_ITConfig(RTC_IT_WUT, ENABLE); //enable interrupt
  RTC_WakeUpCmd(ENABLE);
 
  
  //// Time & date init
  //PWR_RTCAccessCmd(ENABLE);
  RTC_TimeStructInit(&RTC_TimeStruct);
  RTC_SetTime(RTC_Format_BCD,&RTC_TimeStruct);
  RTC_DateStructInit(&RTC_DateStruct);
  RTC_DateStruct.RTC_WeekDay = week_day(RTC_DateStruct.RTC_Date, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Year+2000);
  RTC_SetDate(RTC_Format_BCD,&RTC_DateStruct);
  
  PWR_RTCAccessCmd(DISABLE);
  /*
  RTC_GetDate(RTC_Format_BIN, &RTC_DateStruct);
  RTC_DateStruct.RTC_WeekDay = week_day(RTC_DateStruct.RTC_Date, RTC_DateStruct.RTC_Month, RTC_DateStruct.RTC_Year+2000);
  init_RTC_Date(RTC_Format_BIN, &RTC_DateStruct);
  */
  refresh_time(&RTC_TimeStruct);
  refresh_date(&RTC_DateStruct);
  lcd_SetDash(7,1);
  lcd_SetDash(10,1);
  
  LCD_UpdateDisplayRequest();
  
}


void init_gpio(void)
{
 /* // GPIOA,B,C,D Periph clock enable
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOD, ENABLE);
  
  // Configure PA0,PB6 in output pushpull mode (for light LEDs)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // no resistors
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // no resistors
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // Configure PA1,2,3,6,7,8,9,10,15 for LCD; PA4,5 for buttons
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function GPIO_AF_LCD
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // alternate function EXTI
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_LCD);
  
  
  EXTI_ClearITPendingBit(EXTI_Line4);
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line5);
  EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line11);
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line12);
  EXTI_InitStructure.EXTI_Line = EXTI_Line12;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_EnableIRQ (EXTI4_IRQn);
  NVIC_EnableIRQ (EXTI9_5_IRQn);
  NVIC_EnableIRQ (EXTI15_10_IRQn);

  // Configure PB0,1,3,4,5,8,10,11,12,13,14,15 for LCD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_LCD);
  
  // Configure PB9 for SOUND
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // alternate function TIM4 (AF2)
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_TIM4);
  
  // Configure PC0,1,2,3,4,5,6,7,8,9,10,11,12 for LCD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function GPIO_AF_LCD
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource0, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_LCD);
  
  // Configure PD2 for LCD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function GPIO_AF_LCD
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_LCD);

  //Additional configs
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_7;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // down
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // down
  //GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // down
  GPIO_Init(GPIOH, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // down
  GPIO_Init(GPIOC, &GPIO_InitStructure);*/
  
  
   // GPIOA,B,C,D Periph clock enable
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA|RCC_AHBPeriph_GPIOB|RCC_AHBPeriph_GPIOC|RCC_AHBPeriph_GPIOD, ENABLE);
  
  // Configure PA0,PB6 in output pushpull mode (for light LEDs)
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // no resistors
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // analog to reduce consumption
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // no resistors
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // Configure PA1,2,3,6,7,8,9,10,15 for LCD; PA4,5 for buttons
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function GPIO_AF_LCD
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // alternate function EXTI
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
  GPIO_Init(GPIOA, &GPIO_InitStructure);  
  
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource15, GPIO_AF_LCD);
  
  
  EXTI_ClearITPendingBit(EXTI_Line4);
  EXTI_InitStructure.EXTI_Line = EXTI_Line4;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line5);
  EXTI_InitStructure.EXTI_Line = EXTI_Line5;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line11);
  EXTI_InitStructure.EXTI_Line = EXTI_Line11;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  EXTI_ClearITPendingBit(EXTI_Line12);
  EXTI_InitStructure.EXTI_Line = EXTI_Line12;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);
  NVIC_EnableIRQ (EXTI4_IRQn);
  NVIC_EnableIRQ (EXTI9_5_IRQn);
  NVIC_EnableIRQ (EXTI15_10_IRQn);

  // Configure PB0,1,3,4,5,8,10,11,12,13,14,15 for LCD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_8 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_LCD);
  
  // Configure PB9 for SOUND
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // alternate function TIM4 (AF2)
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOB, &GPIO_InitStructure); 
  
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource9, GPIO_AF_TIM4);
  
  // Configure PC0,1,2,3,4,5,6,7,8,9,10,11,12 for LCD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function GPIO_AF_LCD
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource0, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource9, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_LCD);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_LCD);
  
  // Configure PD2 for LCD
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; // alternate function GPIO_AF_LCD
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_LCD);
  /*RCC->AHBENR |= (RCC_AHBENR_GPIOAEN|RCC_AHBENR_GPIOBEN|RCC_AHBENR_GPIOCEN);
  GPIOA->MODER |= 0x802A00A8;
  GPIOB->MODER |= 0xAAAA0A80;
  GPIOC->MODER |= 0x00AAA0AA;
  GPIOA->OTYPER &=  ~0x0000870E;
  GPIOB->OTYPER &=  ~0x0000FF38;
  GPIOC->OTYPER &=  ~0x00000FCF;
  GPIOA->PUPDR &= ~0xC03F00FC;
  GPIOB->PUPDR &= ~0xFFFF0FC0;
  GPIOC->PUPDR &= ~0x00FFF0FF;
  GPIOA->OSPEEDR &= ~0xC03F00FC;
  GPIOB->OSPEEDR &= ~0xFFFF0FC0;
  GPIOC->OSPEEDR &= ~0xFFFFF0FF;
  
  GPIOA->AFR[0] |= 0x0000BBB0;
  GPIOA->AFR[1] |= 0xB0000BBB;
  GPIOB->AFR[0] |= 0x00BBB000;
  GPIOB->AFR[1] |= 0xBBBBBBBB;
  GPIOC->AFR[0] |= 0xBB00BBBB;
  GPIOC->AFR[1] |= 0x0000BBBB;*/
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_7;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // analog to reduce consumption
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // down
GPIO_Init(GPIOB, &GPIO_InitStructure);
/*
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // analog to reduce consumption
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // down
GPIO_Init(GPIOA, &GPIO_InitStructure);

*/

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // analog to reduce consumption
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN; // down
GPIO_Init(GPIOH, &GPIO_InitStructure);

//GPIO_ResetBits(GPIOH, GPIO_Pin_0 | GPIO_Pin_1);

GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14;
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // analog to reduce consumption
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // push-pull
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz; // the slowest
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL; // down
GPIO_Init(GPIOC, &GPIO_InitStructure);
  
  
  
  
  
  
  
  
}


void init_lcd(void)
{

  LCD_InitStructure.LCD_Bias = LCD_Bias_1_3;
  LCD_InitStructure.LCD_Duty = LCD_Duty_1_3; // multiplex 3
  LCD_InitStructure.LCD_Divider = LCD_Divider_19;
  LCD_InitStructure.LCD_Prescaler = LCD_Prescaler_16;
  LCD_InitStructure.LCD_VoltageSource = LCD_VoltageSource_External;
  LCD_Init(&LCD_InitStructure);
  LCD_MuxSegmentCmd(ENABLE);
  //LCD_ContrastConfig(LCD_Contrast_Level_4);
  //LCD_DeadTimeConfig(LCD_DeadTime_1);
  LCD_Cmd(ENABLE);
 // LCD->CR &= ~LCD_CR_VSEL;
 /* LCD->FCR &= ~LCD_FCR_CC;  
  LCD->FCR |= LCD_FCR_CC_1;*/
  /*RCC->APB1ENR |= RCC_APB1ENR_PWREN|RCC_APB1ENR_LCDEN; 
  PWR->CR |= PWR_CR_DBP; 
  RCC->CSR |= RCC_CSR_RTCRST; 
  RCC->CSR &= ~RCC_CSR_RTCRST; 
  RCC->CSR |= RCC_CSR_LSION; 
  while(!(RCC->CSR&RCC_CSR_LSIRDY)); 
  RCC->CSR |= RCC_CSR_RTCSEL_LSI; */
   /*
  LCD->CR &= ~LCD_CR_BIAS; 
  LCD->CR |= LCD_CR_BIAS_1; 
  
  LCD->CR &=~LCD_CR_DUTY; 
  LCD->CR |= LCD_CR_DUTY_0|LCD_CR_DUTY_1;
  
  //LCD->CR |= LCD_CR_MUX_SEG; 
  
  LCD->FCR &= ~LCD_FCR_PS; 
  LCD->FCR |= (1<<24); 
  
  LCD->FCR &= ~LCD_FCR_DIV; 
  LCD->FCR |= (1<<18); 
  
  LCD->FCR &= ~LCD_FCR_CC;  
  LCD->FCR |= LCD_FCR_CC_1; 
  
  while(!(LCD->SR&LCD_SR_FCRSR)); 
  
  LCD->CR &= ~LCD_CR_VSEL; 
    
  LCD->CR |= LCD_CR_LCDEN; 
  while(!(LCD->SR&LCD_SR_RDY)); 
  while(!(LCD->SR&LCD_SR_ENS));*/
  LCD_blink1=LCD_blink2=LCD_blink3=0xFFFFFFFF;
}


void init_adc(void)
{
    RCC_HSICmd(ENABLE); //ADC!!!!!!!!!!!!!
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    ADC_Cmd(ADC1, ENABLE);
    ADC_InitTypeDef ADC_InitStruct;
    ADC_InitStruct.ADC_Resolution =ADC_Resolution_12b;
    ADC_InitStruct.ADC_ScanConvMode=DISABLE;
    ADC_InitStruct.ADC_ContinuousConvMode=DISABLE;
    ADC_InitStruct.ADC_ExternalTrigConvEdge=ADC_ExternalTrigConvEdge_None;
    //ADC_InitStruct.ADC_ExternalTrigConv;
    ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right; 
    ADC_InitStruct.ADC_NbrOfConversion=1;
    ADC_StructInit(&ADC_InitStruct);
    ADC_TempSensorVrefintCmd(ENABLE);
}

void deinit_adc(void)
{
  ADC_DeInit(ADC1);
  ADC_Cmd(ADC1, DISABLE); 
  RCC_HSICmd(DISABLE);
}