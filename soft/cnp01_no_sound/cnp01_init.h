#ifndef __CNP01_INIT_H
#define __CNP01_INIT_H
#include "stm32l1xx.h"
   
// Init structures
extern GPIO_InitTypeDef        GPIO_InitStructure;
extern RTC_InitTypeDef         RTC_InitStructure; 
extern RTC_TimeTypeDef         RTC_TimeStruct;
extern RTC_DateTypeDef         RTC_DateStruct;
extern NVIC_InitTypeDef        NVIC_InitStructure; 
extern EXTI_InitTypeDef        EXTI_InitStructure;
extern LCD_InitTypeDef         LCD_InitStructure; 

// functions
void init_gpio(void);
void init_lcd(void);
void init_rcc(void);
void init_rtc(void);
ErrorStatus init_RTC_Time(uint32_t RTC_Format, RTC_TimeTypeDef* RTC_TimeStruct);
ErrorStatus init_RTC_Date(uint32_t RTC_Format, RTC_DateTypeDef* RTC_DateStruct);

#endif
