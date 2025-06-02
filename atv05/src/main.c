#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "lcd.h"
#include "spi.h"
#include "adc.h"
#include "max7219.h"



// --- Função para mostrar bits como string com espaços ---
void bits_to_string(uint8_t val, char* str) {
    for (uint8_t i = 0; i < 8; i++) {
        str[i * 2] = (val & (1 << (7 - i))) ? '1' : '0';
        str[i * 2 + 1] = ' ';
    }
    str[16] = '\0';
}

// --- Main ---

int main() {
    spi_init();
    max7219_init();
    max7219_clear();
    adc_init();
    lcd_init();

    int pos_x = 0, pos_y = 0;
    int last_pos_x = -1, last_pos_y = -1;
    char line_bits_str[17];
    char col_bits_str[17];

    while (1) {
        uint16_t adc_x = adc_read(4);
        uint16_t adc_y = adc_read(5);

        // Ajustando a leitura de 0 a 7
        pos_x = (adc_x * 8) / 1024;
        pos_y = (adc_y * 8) / 1024;
        if (pos_x > 7) pos_x = 7;
        if (pos_y > 7) pos_y = 7;
        if (pos_x == 0) pos_x = 1;
        if (pos_y == 0) pos_y = 1;

        if (pos_x != last_pos_x || pos_y != last_pos_y) {
            if (last_pos_y >= 0 && last_pos_y <= 7) {
                max7219_send(last_pos_y + 1, 0x00);
            }

            uint8_t line_val = 1 << pos_x;
            max7219_send(pos_y + 1, line_val);

            uint8_t col_val = 1 << pos_y;

            bits_to_string(line_val, line_bits_str);
            bits_to_string(col_val, col_bits_str);

            lcd_clear();
            lcd_goto(0, 0);
            lcd_print("X:");
            lcd_print(line_bits_str);
            lcd_goto(1, 0);
            lcd_print("Y:");
            lcd_print(col_bits_str);

            last_pos_x = pos_x;
            last_pos_y = pos_y;
        }

        _delay_ms(100);
    }
}