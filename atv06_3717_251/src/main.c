#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "lcd.h"
#include "btn.h"

// Estados da máquina de estados
typedef enum {
    STATE_INITIAL,
    STATE_BUTTONS
} state_t;

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
    
    state_t current_state = STATE_INITIAL;
    
    while(1) {
        switch(current_state) {
            case STATE_INITIAL:
                lcd_clear();
                lcd_goto(0, 0);
                lcd_print("ELE-3717");
                lcd_goto(1, 0);
                lcd_print("FILTRO FIR");
                
                // Aguarda qualquer botão para mudar de estado
                while(!btn_read(BTN_S1) && !btn_read(BTN_S2) && !btn_read(BTN_S3)) {
                    _delay_ms(50);
                }
                current_state = STATE_BUTTONS;
                _delay_ms(300); // Debounce
                break;
                
            case STATE_BUTTONS:
                lcd_clear();
                lcd_goto(0, 0);
                lcd_print("Press buttons:");
                
                while(current_state == STATE_BUTTONS) {
                    if (btn_read(BTN_S1)) {
                        lcd_goto(1, 0);
                        lcd_print("S1 pressed     ");
                        _delay_ms(200);
                    }
                    else if (btn_read(BTN_S2)) {
                        lcd_goto(1, 0);
                        lcd_print("S2 pressed     ");
                        _delay_ms(200);
                    }
                    else if (btn_read(BTN_S3)) {
                        lcd_goto(1, 0);
                        lcd_print("S3 pressed     ");
                        _delay_ms(200);
                    }
                    else {
                        lcd_goto(1, 0);
                        lcd_print("               ");
                    }
                    _delay_ms(50);
                }
                break;
        }
    }
}