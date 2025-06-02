#include "mde.h"
#include "lcd.h"
#include "btn.h"
#include <avr/io.h>
#include <util/delay.h>

static state_t current_state = STATE_INITIAL;

void mde_init(void) {
    current_state = STATE_INITIAL;
}

void mde_run(void) {
    switch(current_state) {
        case STATE_INITIAL:
            lcd_clear();
            // Centraliza "ELE-3717" (8 chars) -> posição (16-8)/2 = 4
            lcd_goto(0, 4);
            lcd_print("ELE-3717");
            // Centraliza "FILTRO FIR" (10 chars) -> posição (16-10)/2 = 3
            lcd_goto(1, 3);
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
            // Centraliza "Press buttons:" (14 chars) -> posição (16-14)/2 = 1
            lcd_goto(0, 1);
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

state_t mde_get_state(void) {
    return current_state;
}

void mde_set_state(state_t new_state) {
    current_state = new_state;
}
