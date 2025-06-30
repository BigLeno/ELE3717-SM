#ifndef LCD_H
#define LCD_H

#include <stdint.h>

void lcd_init(void);
void lcd_clear(void);
void lcd_goto(uint8_t line, uint8_t pos);
void lcd_print(const char *str);
void lcd_print_dec(uint8_t num);

#endif // LCD_H