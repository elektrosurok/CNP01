#ifndef __MAIN_H
#define __MAIN_H

#define SOFT_VERSION 0x00       // In BCD
#define SOFT_RELEASE 0x01       // In BCD

#define DEBUG_MODE      // comment this to make a release version. 
                        // WARNING: SWD will be disabled.

#define STM32L100       // for compat. with this MCU
#define TIM_F 524288

//2097152

void refresh_time();
void refresh_date();
void display();
uint8_t ByteToBcd2(uint8_t );
uint8_t Bcd2ToByte(uint8_t );
uint8_t week_day(uint8_t day, uint8_t month, uint32_t year);
void button_timer();
void button_act_A();
void button_act_B();
void button_act_C();
void new_watch_state (uint8_t state);
uint8_t IncreaseBcd (uint8_t Value);
uint8_t DecreaseBcd (uint8_t Value);
void correction_enable();
void chrono_update ();
void chrono_bcd (uint32_t temp_time);
void split_bcd (uint32_t temp_time);
uint32_t timestamp();
uint32_t difference(uint32_t timestamp1, uint32_t timestamp2);

extern uint8_t tmp;
extern uint8_t action;          // what button is pressed?
extern uint8_t act_tim_state;
extern uint8_t watch_state;     // obviously!
extern uint8_t current_second;  // last showed second
extern uint8_t no_time;         // shows thar time refresh isn't needed
extern int8_t button_cnt[4];   // used to detect button state
extern int8_t sleep_mode;
extern int8_t sleep_sound;
#endif