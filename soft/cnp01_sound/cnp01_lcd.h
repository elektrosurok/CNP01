#ifndef __CNP01_LCD_H
#define __CNP01_LCD_H

#include "stm32l1xx.h"

#define LCD1 LCD->RAM[0]
#define LCD2 LCD->RAM[2]
#define LCD3 LCD->RAM[4]

// Global variables
extern uint32_t LCD_buf1, LCD_buf2, LCD_buf3, LCD_blink1, LCD_blink2, LCD_blink3, LCD_blinkmask;
extern uint8_t LCD_blinkflag,LCD_dashes,LCD_dashes_blink;

// Font for  7-segments symbols, format: 0b0..0gfedcba
extern uint16_t font[10];

// Functions
void lcd_init(void);
void lcd_clear();
void lcd_Set();
void lcd_SetChar(char pos, uint16_t code);
void lcd_SetWord(char num);
void lcd_SetDash(char num, char new_state);
void lcd_SetDashBlink(char num, char new_state);
void lcd_SetAllDashes(uint32_t dashes);
void lcd_SetUpperDashes(char mask);
void lcd_Blink(uint32_t mask);
void lcd_blinker(void);
void lcd_no_blink(void);

#endif