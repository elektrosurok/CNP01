

// Interrupt for button timer
void TIM7_IRQHandler(void)
{
  TIM7->SR &= ~TIM_SR_UIF; //—брасываем флаг UIF
  uint8_t state=0;
  TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
  //if (action&&0x0F)
  //{
    if (action==0x01) state=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5);
    if (action==0x02) state=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_4);
    if (action==0x04) state=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_12);
    if (action==0x08) state=GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_11);
  //}
  //else state=0;
  //if (!state) action=0;
  if (state==Bit_SET)
  {
    switch (act_tim_state)
    {
      case 1:
        button_timer(1024);//TIM_SetAutoreload(TIM7,2000);
        act_tim_state=2;
        button_act_A(); //button action A
        break;
      case 2:
        TIM_SetAutoreload(TIM7,128);
        act_tim_state=3;
        button_act_B(); //button action B
        break;
      case 3:
        //TIM_SetCounter(TIM7,128);
        button_act_C(); //button action C
        break;
    }
    TIM_Cmd(TIM7,ENABLE);
    TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
    NVIC_EnableIRQ(TIM7_IRQn);
    
    /*
    
  TIM_TimeBaseInit(TIM7,&TIM_TimeBaseInitStruct);
  TIM_Cmd(TIM7,ENABLE);
  //TIM_SetCounter(TIM7,64);
  TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);
  NVIC_EnableIRQ(TIM7_IRQn);
    */
  }
  if (!state)
  {
    TIM_ITConfig(TIM7, TIM_IT_Update, DISABLE);
    TIM_DeInit(TIM7);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
    action=0;
    act_tim_state=0;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7,DISABLE);
  }
}
  