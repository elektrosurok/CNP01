//
#include "cnp01_snd.h"
#include "cnp01_init.h"
#include "main.h"

enum notes {pause,c1,cs1,d1,ds1,e1,f1,fs1,g1,gs1,a1,b1,h1,c2,cs2,d2,ds2,e2,f2,fs2,g2,gs2,a2,b2,h2,c3,cs3,d3,ds3,e3,f3,fs3,g3,gs3,a3,b3,h3,c4};
const uint16_t note_p[]={0, 977, 924, 871, 823, 776, 734, 692, 653, 617, 582, 549, 518, 489, 462, 436, 412, 388, 367, 346, 327, 308, 291, 275, 259, 245, 231, 218, 206, 194, 183, 173, 163, 154, 145, 137, 130, 122, 0};
//нота, длительность, ... , 0, 0, ...
const uint8_t melody[]={h1,4,a1,4,gs1,4,a1,4,c2,16,d2,4,c2,4,h1,4,c2,4,e2,16,f2,4,e2,4,ds2,4,e2,4,h2,4,a2,4,gs2,4,a2,4,h2,4,a2,4,gs2,4,a2,4,c3,16,a2,8,c3,6,g2,1,a2,1,h2,8,a2,8,g2,8,a2,6,
g2,1,a2,1,h2,8,a2,8,g2,8,a2,6,g2,1,a2,1,h2,8,a2,8,g2,8,fs2,8,e2,16,h1,4,a1,4,gs1,4,a1,4,c2,16,d2,4,c2,4,h1,4,c2,4,e2,16,f2,4,e2,4,ds2,4,e2,4,h2,4,a2,4,gs2,4,a2,4,h2,4,a2,4,gs2,4,a2,4,c3,16,
a2,8,h2,8,c3,8,h2,8,a2,8,gs2,8,a2,8,e2,8,f2,8,d2,8,c2,16,c2,2,h1,2,c2,2,h1,2,c2,2,h1,2,a1,2,h1,2,a1,32,0,0,

h2,8,c3,8,h2,8,g2,8,e2,8,h2,8,g2,8,h2,8,h1,8,h2,8,g2,8,h2,8, a2,8,h2,8,a2,8,fs2,8,ds2,8,a2,8,fs2,8,a2,8,h1,8,a2,8,fs2,8,a2,8,
g2,8,a2,8,g2,8,e2,8,h1,8,g2,8,fs2,8,g2,8,fs2,8,c2,8,a1,8,fs2,8,e2,8,fs2,8,e2,8,cs2,8,g1,8,e2,8,ds2,8,h1,8,b2,8,h1,8,h2,8,h1,8,
h2,8,c3,8,h2,8,g2,8,e2,8,h2,8,g2,8,h2,8,h1,8,h2,8,g2,8,h2,8, a2,8,h2,8,a2,8,fs2,8,ds2,8,a2,8,fs2,8,a2,8,h1,8,a2,8,fs2,8,a2,8,
g2,8,a2,8,g2,8,e2,8,h1,8,g2,8,fs2,8,g2,8,fs2,8,c2,8,a1,8,fs2,8,ds2,8,e2,8,ds2,8,h1,8,fs1,8,ds2,8,e2,32,0,0,

e2,2,0,0,

e2,16,0,16,e2,16,0,16,e2,16,0,16,e2,16,0,16,0,0};
uint32_t position=0;
uint8_t playing=0;
uint8_t note,length;

uint32_t determine_start_pos (uint32_t m_number)
{
   uint32_t tmp1=0;
   uint32_t tmp2=0;
   while(tmp2<m_number)
   {
     if ((melody[tmp1]==0)&&(melody[tmp1+1]==0)) tmp2++;
     tmp1=tmp1+2;
   }
   return tmp1;
   
}

void play (uint32_t m_number)
{
   tim11_deinit();
   position=determine_start_pos(m_number);
   playing=1;
   sleep_sound=1;
   tim11_init(1);
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
    
}

void sound_deinit (void)
{
  TIM_Cmd(TIM4,DISABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,DISABLE);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN; // alternate function TIM4 (AF2) OFF!
     GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_400KHz;
     GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
     GPIO_Init(GPIOB, &GPIO_InitStructure);
  //sleep_mode=0;
}

void tim11_init (uint32_t period)
{
          RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11,ENABLE);
          TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
          TIM_TimeBaseInitStruct.TIM_Prescaler=1<<8 - 1;
          TIM_TimeBaseInitStruct.TIM_CounterMode=TIM_CounterMode_Up;
          TIM_TimeBaseInitStruct.TIM_Period=(period<<3)-1;
          TIM_TimeBaseInitStruct.TIM_ClockDivision=TIM_CKD_DIV1;
          TIM_TimeBaseInit(TIM11,&TIM_TimeBaseInitStruct);
          TIM_ETRClockMode2Config(TIM11, TIM_ExtTRGPSC_OFF, TIM_ExtTRGPolarity_NonInverted, 0);
          TIM11->SR =0;//&= ~TIM_SR_UIF;
          //TIM11->CNT=0;
          TIM_ITConfig(TIM11, TIM_IT_Update, ENABLE);
          NVIC_EnableIRQ(TIM11_IRQn);
          TIM_Cmd(TIM11,ENABLE);
          //sleep_mode=1; //##############################################
}

void tim11_deinit (void)
{
  TIM11->SR &= ~TIM_SR_UIF;
  NVIC_DisableIRQ(TIM11_IRQn);
  TIM_ITConfig(TIM11, TIM_IT_Update, DISABLE);
  TIM_Cmd(TIM11,DISABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM11,DISABLE);
}

void TIM11_IRQHandler(void)
{
  
  note=melody[position];
  length=melody[position+1];
  if (note==0) sound_deinit(); else sound_init(note_p[note]);
 
  if (length==0) {
    stop();
  } 
  else tim11_init(length);
  position=position+2; 
   TIM11->SR &= ~TIM_SR_UIF;
   //—брасываем флаг UIF
  
}

void stop(void)
{
  tim11_deinit();sound_deinit();sleep_sound=0;
  playing=0;
}