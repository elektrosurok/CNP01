#ifndef __CNP01_SND_H
#define __CNP01_SND_H
#include "stm32l1xx.h"

extern uint8_t playing;


void sound_init (uint32_t period);
void sound_deinit (void);
void tim11_init (uint32_t period);
void tim11_deinit (void);
void play (uint32_t m_number);
void stop (void);
uint32_t determine_start_pos (uint32_t m_number);

#endif