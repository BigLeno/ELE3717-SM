#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "lcd.h"

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
    lcd_print("Hello World");
    
    while(1) {
        // Loop infinito
    }
}