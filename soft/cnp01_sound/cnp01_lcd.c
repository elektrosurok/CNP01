#include "cnp01_lcd.h"
#include "main.h"

// NOTE!
// Positions are numbered from left-top to right-bottom corner
// π1,2 - 7-seg + 2 add.
// π3 - only "L" character
// π4,10 - 7-seg, but a,d segments are common
// π5,6,7,9,11,12,13 - 7-seg
// π8 - only "1" or "2"
// π14 - "'" and ":" (8 segments): ...M1,M2-3,M4,M5-6,J7,H1-2,H3-4,H5-7
// π15 - mode signs (8 segments): ...J1,J2,J3,J4,J5,J6

// Global variables
uint32_t LCD_buf1, LCD_buf2, LCD_buf3, LCD_blink1, LCD_blink2, LCD_blink3, LCD_blinkmask;
uint8_t LCD_blinkflag,LCD_dashes,LCD_dashes_blink;

// Font for  7-segments symbols, format: 0b0..0gfedcba
uint16_t font[10]={
  0x3F, // 0
  0x06, // 1
  0x5B, // 2
  0x4F, // 3
  0x66, // 4
  0x6D, // 5
  0x7D, // 6
  0x07, // 7
  0x7F, // 8
  0x6F, // 9
  
   
  };


void lcd_Set()
{
  LCD1=LCD_buf1;
  LCD2=LCD_buf2;
  LCD3=LCD_buf3;
}

void lcd_SetChar(char pos, uint16_t code)
{
  switch (pos)
  {
    case 1:
      LCD_buf1&=~(uint32_t)((1<<27)|(1<<17)|(1<<28));
      LCD_buf2&=~(uint32_t)((1<<28)|(1<<17));
      LCD_buf3&=~(uint32_t)((1<<27)|(1<<17)|(1<<28));
      if (code&(1<<0)) LCD_buf3|=1<<17; // seg a
      if (code&(1<<1)) LCD_buf3|=1<<27; // seg b      
      if (code&(1<<2)) LCD_buf1|=1<<27; // seg c
      if (code&(1<<3)) LCD_buf1|=1<<28; // seg d      
      if (code&(1<<4)) LCD_buf2|=1<<28; // seg e
      if (code&(1<<5)) LCD_buf3|=1<<28; // seg f
      if (code&(1<<6)) LCD_buf1|=1<<17; // seg g
      break;
    case 2:
      LCD_buf1&=~(uint32_t)((1<<15)|(1<<26)|(1<<24)|(1<<25));
      //LCD_buf2 - no segments
      LCD_buf3&=~(uint32_t)((1<<24)|(1<<15)|(1<<26)|(1<<25));
      if (code&(1<<0)) LCD_buf3|=1<<24; // seg a
      if (code&(1<<1)) LCD_buf3|=1<<15; // seg b      
      if (code&(1<<2)) LCD_buf1|=1<<15; // seg c
      if (code&(1<<3)) LCD_buf1|=1<<26; // seg d      
      if (code&(1<<4)) LCD_buf3|=1<<26; // seg e
      if (code&(1<<5)) LCD_buf3|=1<<25; // seg f
      if (code&(1<<6)) LCD_buf1|=1<<24; // seg g 
      break;
    case 4:  // "7" CANNOT be displayed!
      LCD_buf1&=~(uint32_t)((1<<12)|(1<<13)|(1<<14));
      LCD_buf2&=~(uint32_t)((1<<13));
      LCD_buf3&=~(uint32_t)((1<<13)|(1<<12));
      if (code&(1<<0)) LCD_buf2|=1<<13; // seg a (=d)
      if (code&(1<<1)) LCD_buf3|=1<<12; // seg b      
      if (code&(1<<2)) LCD_buf1|=1<<12; // seg c
      //if (code&(1<<3)) LCD_buf2|=1<<13; // seg d (=a)   
      if (code&(1<<4)) LCD_buf1|=1<<14; // seg e
      if (code&(1<<5)) LCD_buf3|=1<<13; // seg f
      if (code&(1<<6)) LCD_buf1|=1<<13; // seg g
      break;
    case 5:
      LCD_buf1&=~(uint32_t)((1<<6)|(1<<10)|(1<<11));
      LCD_buf2&=~(uint32_t)((1<<10));
      LCD_buf3&=~(uint32_t)((1<<6)|(1<<11)|(1<<10));
      if (code&(1<<0)) LCD_buf2|=1<<10; // seg a
      if (code&(1<<1)) LCD_buf3|=1<<6; // seg b      
      if (code&(1<<2)) LCD_buf1|=1<<6; // seg c
      if (code&(1<<3)) LCD_buf1|=1<<10; // seg d   
      if (code&(1<<4)) LCD_buf1|=1<<11; // seg e
      if (code&(1<<5)) LCD_buf3|=1<<11; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<10; // seg g
      break;
    case 6: 
      LCD_buf1&=~(uint32_t)((1<<4)|(1<<22)|(1<<23));
      LCD_buf2&=~(uint32_t)((1<<23)|(1<<22));
      LCD_buf3&=~(uint32_t)((1<<23)|(1<<22));
      if (code&(1<<0)) LCD_buf2|=1<<23; // seg a
      if (code&(1<<1)) LCD_buf2|=1<<22; // seg b      
      if (code&(1<<2)) LCD_buf1|=1<<4; // seg c
      if (code&(1<<3)) LCD_buf1|=1<<22; // seg d   
      if (code&(1<<4)) LCD_buf1|=1<<23; // seg e
      if (code&(1<<5)) LCD_buf3|=1<<23; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<22; // seg g
      break;
    case 7:
      LCD_buf1&=~(uint32_t)((1<<2)|(1<<3));
      LCD_buf2&=~(uint32_t)((1<<3)|(1<<2));
      LCD_buf3&=~(uint32_t)((1<<2)|(1<<4)|(1<<3));
      if (code&(1<<0)) LCD_buf2|=1<<3; // seg a
      if (code&(1<<1)) LCD_buf2|=1<<2; // seg b      
      if (code&(1<<2)) LCD_buf3|=1<<2; // seg c
      if (code&(1<<3)) LCD_buf1|=1<<2; // seg d   
      if (code&(1<<4)) LCD_buf1|=1<<3; // seg e
      if (code&(1<<5)) LCD_buf3|=1<<4; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<3; // seg g
      break;
    case 8: // ONLY "1" and "2" can be displayed!
      LCD_buf1&=~(uint32_t)((1<<29));
      LCD_buf2&=~(uint32_t)((1<<29));
      LCD_buf3&=~(uint32_t)((1<<29));
      if (code==font[0]) break;
      if (code&(1<<0)) LCD_buf1|=1<<29; // seg a,d,e,g
      if (code&(1<<1)) LCD_buf3|=1<<29; // seg b
      if (code&(1<<2)) LCD_buf2|=1<<29; // seg c      
      break;
    case 9: 
      LCD_buf1&=~(uint32_t)((1<<7)|(1<<31)|(1<<30));
      LCD_buf2&=~(uint32_t)((1<<31)|(1<<30));
      LCD_buf3&=~(uint32_t)((1<<31)|(1<<30));
      if (code&(1<<0)) LCD_buf1|=1<<31; // seg a
      if (code&(1<<1)) LCD_buf1|=1<<7; // seg b      
      if (code&(1<<2)) LCD_buf2|=1<<31; // seg c
      if (code&(1<<3)) LCD_buf2|=1<<30; // seg d   
      if (code&(1<<4)) LCD_buf3|=1<<30; // seg e
      if (code&(1<<5)) LCD_buf1|=1<<30; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<31; // seg g
      break;
    case 10: // "7" CANNOT be displayed!
      LCD_buf1&=~(uint32_t)((1<<9)|(1<<8));
      LCD_buf2&=~(uint32_t)((1<<8)|(1<<7));
      LCD_buf3&=~(uint32_t)((1<<9)|(1<<8));
      if (code&(1<<0)) LCD_buf2|=1<<8; // seg a (=d)
      if (code&(1<<1)) LCD_buf1|=1<<9; // seg b      
      if (code&(1<<2)) LCD_buf3|=1<<9; // seg c
      //if (code&(1<<3)) LCD_buf2|=1<<8; // seg d   
      if (code&(1<<4)) LCD_buf2|=1<<7; // seg e
      if (code&(1<<5)) LCD_buf1|=1<<8; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<8; // seg g
      break;
    case 11:
      LCD_buf1&=~(uint32_t)((1<<18)|(1<<16));
      LCD_buf2&=~(uint32_t)((1<<18)|(1<<16)|(1<<9));
      LCD_buf3&=~(uint32_t)((1<<18)|(1<<16));
      if (code&(1<<0)) LCD_buf1|=1<<18; // seg a
      if (code&(1<<1)) LCD_buf3|=1<<18; // seg b      
      if (code&(1<<2)) LCD_buf2|=1<<18; // seg c
      if (code&(1<<3)) LCD_buf2|=1<<16; // seg d   
      if (code&(1<<4)) LCD_buf2|=1<<9; // seg e
      if (code&(1<<5)) LCD_buf1|=1<<16; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<16; // seg g
      break;
    case 12: 
      LCD_buf1&=~(uint32_t)((1<<20)|(1<<21)|(1<<19));
      LCD_buf2&=~(uint32_t)((1<<20)|(1<<19));
      LCD_buf3&=~(uint32_t)((1<<19)|(1<<20));
      if (code&(1<<0)) LCD_buf1|=1<<20; // seg a
      if (code&(1<<1)) LCD_buf1|=1<<21; // seg b      
      if (code&(1<<2)) LCD_buf2|=1<<20; // seg c
      if (code&(1<<3)) LCD_buf2|=1<<19; // seg d   
      if (code&(1<<4)) LCD_buf3|=1<<19; // seg e
      if (code&(1<<5)) LCD_buf1|=1<<19; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<20; // seg g
      break;
    case 13:
      LCD_buf1&=~(uint32_t)((1<<0));
      LCD_buf2&=~(uint32_t)((1<<0)|(1<<1)|(1<<21));
      LCD_buf3&=~(uint32_t)((1<<1)|(1<<0)|(1<<21));
      if (code&(1<<0)) LCD_buf1|=1<<0; // seg a
      if (code&(1<<1)) LCD_buf3|=1<<1; // seg b      
      if (code&(1<<2)) LCD_buf2|=1<<1; // seg c
      if (code&(1<<3)) LCD_buf2|=1<<0; // seg d   
      if (code&(1<<4)) LCD_buf2|=1<<21; // seg e
      if (code&(1<<5)) LCD_buf3|=1<<21; // seg f
      if (code&(1<<6)) LCD_buf3|=1<<0; // seg g
      break;
    default: 
      break;
  }
}
    
void lcd_SetWord(char num)
{
  LCD_buf1&=~(uint32_t)((1<<27)|(1<<17)|(1<<28)|(1<<15)|(1<<26)|(1<<24)|(1<<25));
  LCD_buf2&=~(uint32_t)((1<<28)|(1<<17)|(1<<26)|(1<<32));
  LCD_buf3&=~(uint32_t)((1<<14)|(1<<27)|(1<<17)|(1<<28)|(1<<24)|(1<<15)|(1<<26)|(1<<25));
  switch (num)
  {
    case 1: //œÕ (1)
      LCD_buf1|=(1<<27)|(1<<15)|(1<<24); 
      LCD_buf2|=1<<28;
      LCD_buf3|=(1<<17)|(1<<27)|(1<<28)|(1<<15)|(1<<26)|(1<<25);
      break;
    case 2: //¬“ (2)   
      LCD_buf1|=(1<<17)|(1<<27)|(1<<28);     
      LCD_buf2|=(1<<28)|(1<<26);
      LCD_buf3|=(1<<17)|(1<<27)|(1<<28)|(1<<24)|(1<<26)|(1<<25);
      break;
    case 3: //—– (3)   
      LCD_buf1|=(1<<28)|(1<<24);
      LCD_buf2|=(1<<28);
      LCD_buf3|=(1<<17)|(1<<28)|(1<<24)|(1<<15)|(1<<26)|(1<<25);
      break;
    case 4: //◊“ (4)  
      LCD_buf1|=(1<<27)|(1<<17);
      LCD_buf2|=1<<26; //L1
      LCD_buf3|=(1<<27)|(1<<28)|(1<<24)|(1<<26)|(1<<25);
      break;
    case 5: //œ“ (5)     
      LCD_buf1|=(1<<27);    
      LCD_buf2|=(1<<28)|(1<<26);
      LCD_buf3|=(1<<17)|(1<<27)|(1<<28)|(1<<24)|(1<<26)|(1<<25);
      break;
    case 6: //—¡ (6)   
      LCD_buf1|=(1<<28)|(1<<15)|(1<<26)|(1<<24); 
      LCD_buf2|=1<<28;
      LCD_buf3|=(1<<17)|(1<<28)|(1<<24)|(1<<26)|(1<<25); // seg f
      break;
    case 7: //¬— (7)   
      LCD_buf1|=(1<<27)|(1<<28)|(1<<17)|(1<<26);      
      LCD_buf2|=1<<28;
      LCD_buf3|=(1<<28)|(1<<17)|(1<<27)|(1<<24)|(1<<26)|(1<<25);
      break;
    case 8: //—√ (8)   
      LCD_buf1|=(1<<28); 
      LCD_buf2|=1<<28;
      LCD_buf3|=(1<<17)|(1<<28)|(1<<24)|(1<<26)|(1<<25);
      break;
    case 9: //—K (9)   
      LCD_buf1|=(1<<28)|(1<<24)|(1<<25); 
      LCD_buf2|=1<<28;
      LCD_buf3|=(1<<17)|(1<<15)|(1<<28)|(1<<26)|(1<<25);
      break;
    case 10: //T (10)   
      LCD_buf1|=1<<27; 
      LCD_buf2|=1<<26;
      LCD_buf3|=(1<<17)|(1<<27); // seg f
      break;
    case 11: //AL (11)   
      LCD_buf1|=(1<<15)|(1<<24); 
      LCD_buf3|=(1<<14)|(1<<24)|(1<<15|(1<<25)|(1<<26)); //
      break;
    case 12: //ST (12)   
      LCD_buf1|=(1<<28)|(1<<17)|(1<<27);
      LCD_buf2|=(1<<26);
      LCD_buf3|=(1<<17)|(1<<28)|(1<<24)|(1<<26)|(1<<25);
      break;
    case 13: //Õ“ (13)   
      LCD_buf1|=(1<<27)|(1<<17);    
      LCD_buf2|=(1<<28)|(1<<26);
      LCD_buf3|=(1<<27)|(1<<28)|(1<<24)|(1<<26)|(1<<25);
      break;
  }
}

// mask: 0b00123456
void lcd_SetUpperDashes(char mask)
{
  if (mask&0x20) LCD_buf2|=1<<27; //J1
  else LCD_buf2&=~(uint32_t)(1<<27);
  if (mask&0x20) LCD_buf2|=1<<25; //J2
  else LCD_buf2&=~(uint32_t)(1<<25);
  if (mask&0x20) LCD_buf2|=1<<27; //J3
  else LCD_buf2&=~(uint32_t)(1<<27);
  if (mask&0x20) LCD_buf2|=1<<11; //J4
  else LCD_buf2&=~(uint32_t)(1<<11);
  if (mask&0x20) LCD_buf2|=1<<5; //J5
  else LCD_buf2&=~(uint32_t)(1<<5);
  if (mask&0x20) LCD_buf2|=1<<4; //J6
  else LCD_buf2&=~(uint32_t)(1<<4);
}

void lcd_SetDash(char num, char new_state)
{
  if (new_state)
    switch (num)
    {
      case 1: LCD_buf2|=1<<27; //J1
        break;
      case 2: LCD_buf2|=1<<25;
        break;
      case 3: LCD_buf2|=1<<24;
        break;
      case 4: LCD_buf2|=1<<11;
        break;
      case 5: LCD_buf2|=1<<5;
        break;
      case 6: LCD_buf2|=1<<4; //J6
        break;
      case 7: LCD_buf3|=1<<5; //J7
        break;
      case 8: LCD_buf2|=1<<14; //H1,2
        break;
      case 9: LCD_buf1|=1<<5; //H3,4
        break;
      case 10: LCD_buf3|=1<<7; //H5,6
        break;
      case 11: LCD_buf2|=1<<15; //M1
        break;
      case 12: LCD_buf2|=1<<6; //M2,3
        break;
      case 13: LCD_buf2|=1<<12; //M4
        break;
      case 14: LCD_buf1|=1<<1; //M5,6
        break;
      case 15: LCD_buf3|=1<<14; //L
      break;
    }
  else
    switch (num)
    {
      case 1: LCD_buf2&=~((uint32_t)(1<<27)&(LCD_blink2)); //J1
        break;
      case 2: LCD_buf2&=~((uint32_t)(1<<25)&(LCD_blink2));
        break;
      case 3: LCD_buf2&=~((uint32_t)(1<<24)&(LCD_blink2));
        break;
      case 4: LCD_buf2&=~((uint32_t)(1<<11)&(LCD_blink2));
        break;
      case 5: LCD_buf2&=~((uint32_t)(1<<5)&(LCD_blink2));
        break;
      case 6: LCD_buf2&=~((uint32_t)(1<<4)&(LCD_blink2)); //J6
        break;
      case 7: LCD_buf3&=~(uint32_t)(1<<5); //J7
        break;
      case 8: LCD_buf2&=~(uint32_t)(1<<14); //H1,2
        break;
      case 9: LCD_buf1&=~(uint32_t)(1<<5); //H3,4
        break;
      case 10: LCD_buf3&=~(uint32_t)(1<<7); //H5,6
        break;
      case 11: LCD_buf2&=~(uint32_t)(1<<15); //M1
        break;
      case 12: LCD_buf2&=~(uint32_t)(1<<6); //M2,3
        break;
      case 13: LCD_buf2&=~(uint32_t)(1<<12); //M4
        break;
      case 14: LCD_buf1&=~(uint32_t)(1<<1); //M5,6
        break;
      case 15: LCD_buf3&=~(uint32_t)(1<<14); //L
        break;
  }
}

void lcd_SetDashBlink(char num, char new_state)
{
  if (new_state)
  {
    LCD_dashes_blink|=1<<num;
    switch (num)
    {
      case 1: LCD_blink2&=~(uint32_t)(1<<27); //J1
        break;
      case 2: LCD_blink2&=~(uint32_t)(1<<25);
        break;
      case 3: LCD_blink2&=~(uint32_t)(1<<24);
        break;
      case 4: LCD_blink2&=~(uint32_t)(1<<11);
        break;
      case 5: LCD_blink2&=~(uint32_t)(1<<5);
        break;
      case 6: LCD_blink2&=~(uint32_t)(1<<4); //J6
        break;
    }
  }
  else
  {
    LCD_dashes_blink&=~(1<<num);
    switch (num)
    {
      case 1: LCD_blink2|=1<<27; //J1
        break;
      case 2: LCD_blink2|=1<<25;
        break;
      case 3: LCD_blink2|=1<<24;
        break;
      case 4: LCD_blink2|=1<<11;
        break;
      case 5: LCD_blink2|=1<<5;
        break;
      case 6: LCD_blink2|=1<<4; //J6
        break;
    }
  }
}

void lcd_SetAllDashes(uint32_t dashes)
{
  uint8_t i;
  for(i=7;i<16;i++)
  {
    lcd_SetDash(i,(dashes&(1<<i))>>i);
  }
}


void lcd_Blink(uint32_t mask) 
{
  LCD_blinkmask = mask;
  LCD_blink1|=0xFFFFFFFF;
  LCD_blink2|=0xFFFFFFFF&(~(uint32_t)((1<<27)|(1<<25)|(1<<24)|(1<<11)|(1<<5)|(1<<4)));
  LCD_blink3|=0xFFFFFFFF;
  if (mask)
  {
    if (mask&(1<<12)) 
    {
      LCD_blink1&=~(uint32_t)((1<<27)|(1<<17)|(1<<28));
      LCD_blink2&=~(uint32_t)((1<<28)|(1<<17));
      LCD_blink3&=~(uint32_t)((1<<27)|(1<<17)|(1<<28));
    }
    if (mask&(1<<11)) 
    {
      LCD_blink1&=~(uint32_t)((1<<15)|(1<<26)|(1<<24)|(1<<25));
      LCD_blink2&=~(uint32_t)(1<<26);
      LCD_blink3&=~(uint32_t)((1<<24)|(1<<15)|(1<<26)|(1<<25));
    }
    if (mask&(1<<10)) 
    {
      LCD_blink3&=~(uint32_t)((1<<14));
    }
    if (mask&(1<<9)) 
    {
      LCD_blink1&=~(uint32_t)((1<<12)|(1<<13)|(1<<14));
      LCD_blink2&=~(uint32_t)((1<<13));
      LCD_blink3&=~(uint32_t)((1<<13)|(1<<12));
    }
    if (mask&(1<<8)) 
    {
      LCD_blink1&=~(uint32_t)((1<<6)|(1<<10)|(1<<11));
      LCD_blink2&=~(uint32_t)((1<<10));
      LCD_blink3&=~(uint32_t)((1<<6)|(1<<11)|(1<<10));
    }
    if (mask&(1<<7)) 
    {
      LCD_blink1&=~(uint32_t)((1<<4)|(1<<22)|(1<<23));
      LCD_blink2&=~(uint32_t)((1<<23)|(1<<22));
      LCD_blink3&=~(uint32_t)((1<<23)|(1<<22));
    }
    if (mask&(1<<6)) 
    {
      LCD_blink1&=~(uint32_t)((1<<2)|(1<<3));
      LCD_blink2&=~(uint32_t)((1<<3)|(1<<2));
      LCD_blink3&=~(uint32_t)((1<<2)|(1<<4)|(1<<3));
    }
    if (mask&(1<<5)) 
    {
      LCD_blink1&=~(uint32_t)((1<<29));
      LCD_blink2&=~(uint32_t)((1<<29));
      LCD_blink3&=~(uint32_t)((1<<29));
    }
    if (mask&(1<<4)) 
    {
      LCD_blink1&=~(uint32_t)((1<<7)|(1<<31)|(1<<30));
      LCD_blink2&=~(uint32_t)((1<<31)|(1<<30));
      LCD_blink3&=~(uint32_t)((1<<31)|(1<<30));
    }
    if (mask&(1<<3)) 
    {
      LCD_blink1&=~(uint32_t)((1<<9)|(1<<8));
      LCD_blink2&=~(uint32_t)((1<<8)|(1<<7));
      LCD_blink3&=~(uint32_t)((1<<9)|(1<<8));
    }
    if (mask&(1<<2)) 
    {
      LCD_blink1&=~(uint32_t)((1<<18)|(1<<16));
      LCD_blink2&=~(uint32_t)((1<<18)|(1<<16)|(1<<9));
      LCD_blink3&=~(uint32_t)((1<<18)|(1<<16));
    }
    if (mask&(1<<1)) 
    {
      LCD_blink1&=~(uint32_t)((1<<20)|(1<<21)|(1<<19));
      LCD_blink2&=~(uint32_t)((1<<20)|(1<<19));
      LCD_blink3&=~(uint32_t)((1<<19)|(1<<20));
    }
    if (mask&(1<<0)) 
    {
      LCD_blink1&=~(uint32_t)((1<<0));
      LCD_blink2&=~(uint32_t)((1<<0)|(1<<1)|(1<<21));
      LCD_blink3&=~(uint32_t)((1<<1)|(1<<0)|(1<<21));
    }
    // Initializing timer
/*    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6,ENABLE);
    
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_TimeBaseInitStruct.TIM_Prescaler=TIM_F/256 - 1;
    TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period=64; // Original watch blinks 2 times a sec
    TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM6,&TIM_TimeBaseInitStruct);
    TIM_SetCounter(TIM6,32);
    
    // Initializing interrupt
    TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM6,ENABLE);
    NVIC_EnableIRQ(TIM6_IRQn);
    
    
    LCD_blinkflag=1;
    
  }
  else
  {
    // Deinitialize timer
    TIM_DeInit(TIM6);
  };*/
  }
}

  

// Interrupt for blinking
void lcd_blinker()
{
  //TIM6->SR &= ~TIM_SR_UIF; //—·‡Ò˚‚‡ÂÏ ÙÎ‡„ UIF
  lcd_Set();
  if (LCD_blinkflag)
  {
    //LCD_buf1=LCD1;
    //LCD_buf2=LCD2;
    //LCD_buf3=LCD3;
    LCD1&=LCD_blink1;
    LCD2&=LCD_blink2;
    LCD3&=LCD_blink3;
    LCD_blinkflag=0;
  }
  else
  {
    LCD_blinkflag=1;
  }
  LCD_UpdateDisplayRequest();
}

void lcd_no_blink()
 {
  lcd_Set();
  LCD_UpdateDisplayRequest();
 }

// Clears all LCD except dashes
void lcd_clear()
{
  LCD_buf1&=0;
  LCD_buf2&=(1<<27)|(1<<25)|(1<<24)|(1<<11)|(1<<5)|(1<<4);
  LCD_buf3&=0;
}