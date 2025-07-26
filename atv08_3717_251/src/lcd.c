#include "lcd.h"
#include <avr/io.h>
#include <util/delay.h>

// Definições dos pinos e portas do LCD
#define LCD_PORT PORTD
#define LCD_DDR DDRD
#define RS PD2
#define E  PD3

static void lcd_pulse_enable(void) {
    LCD_PORT |= (1 << E);
    _delay_us(1);
    LCD_PORT &= ~(1 << E);
    _delay_us(100);
}

static void lcd_send_nibble(uint8_t nibble) {
    LCD_PORT &= 0x0F;           // limpando os 4 bits altos
    LCD_PORT |= (nibble << 4);  // setando os 4 bits altos conforme nibble
    lcd_pulse_enable();
}

static void lcd_send_byte(uint8_t data, uint8_t is_data) {
    if (is_data)
        LCD_PORT |= (1 << RS);
    else
        LCD_PORT &= ~(1 << RS);

    lcd_send_nibble(data >> 4);
    lcd_send_nibble(data & 0x0F);
    _delay_us(50);
}

void lcd_init(void) {
    LCD_DDR |= (1 << RS) | (1 << E) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7);
    _delay_ms(40);
    LCD_PORT &= ~((1 << RS) | (1 << E) | (1 << PD4) | (1 << PD5) | (1 << PD6) | (1 << PD7));

    lcd_send_nibble(0x03);
    _delay_ms(5);
    lcd_send_nibble(0x03);
    _delay_us(150);
    lcd_send_nibble(0x03);
    lcd_send_nibble(0x02);

    lcd_send_byte(0x28, 0); // 4 bits, 2 linhas, fonte 5x8
    lcd_send_byte(0x0C, 0); // Display ligado, cursor desligado
    lcd_send_byte(0x06, 0); // Incrementa cursor
    lcd_send_byte(0x01, 0); // Limpa display
    _delay_ms(2);
}

void lcd_clear(void) {
    lcd_send_byte(0x01, 0);
    _delay_ms(2);
}

void lcd_goto(uint8_t line, uint8_t pos) {
    uint8_t addr = pos + (line ? 0x40 : 0x00);
    lcd_send_byte(0x80 | addr, 0);
}

void lcd_print(const char *str) {
    while (*str) {
        lcd_send_byte(*str++, 1);
    }
}

void lcd_print_dec(uint8_t num) {
    char buf[4];
    int i = 0;
    if (num == 0) {
        lcd_send_byte('0', 1);
        return;
    }
    while (num > 0 && i < 3) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    for (int j = i - 1; j >= 0; j--) {
        lcd_send_byte(buf[j], 1);
    }
}