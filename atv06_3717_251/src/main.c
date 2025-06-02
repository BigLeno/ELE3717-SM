#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "lcd.h"
#include "btn.h"



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
    lcd_init();
    btn_init();
    
    lcd_print("Hello World");
    lcd_goto(1, 0);
    lcd_print("Press buttons:");
    
    while(1) {
        if (btn_read(BTN_S1)) {
            lcd_goto(1, 14);
            lcd_print("S1");
            _delay_ms(200);
        }
        else if (btn_read(BTN_S2)) {
            lcd_goto(1, 14);
            lcd_print("S2");
            _delay_ms(200);
        }
        else if (btn_read(BTN_S3)) {
            lcd_goto(1, 14);
            lcd_print("S3");
            _delay_ms(200);
        }
        else {
            lcd_goto(1, 14);
            lcd_print("  ");
        }
    }
}