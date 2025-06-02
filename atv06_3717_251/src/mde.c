#include "mde.h"
#include "lcd.h"
#include "btn.h"
#include <avr/io.h>
#include <util/delay.h>

static state_t current_state = STATE_INITIAL;
static uint8_t coef_index = 0;
static uint8_t last_btn_state = 0;

void mde_init(void) {
    current_state = STATE_INITIAL;
    coef_index = 0;
}

void mde_run(void) {
    uint8_t btn_s3_pressed = btn_read(BTN_S3);
    
    // Detecta borda de subida do botão S3
    if (btn_s3_pressed && !last_btn_state) {
        switch(current_state) {
            case STATE_INITIAL:
                current_state = STATE_COEFFICIENTS;
                coef_index = 0;
                break;
            case STATE_BUTTONS:
                current_state = STATE_COEFFICIENTS;
                coef_index = 0;
                break;
            case STATE_COEFFICIENTS:
                coef_index++;
                if (coef_index > 15) {
                    current_state = STATE_INITIAL;
                    coef_index = 0;
                }
                break;
        }
        _delay_ms(300); // Debounce
    }
    last_btn_state = btn_s3_pressed;
    
    switch(current_state) {
        case STATE_INITIAL:
            lcd_clear();
            // Centraliza "ELE-3717" (8 chars) -> posição (16-8)/2 = 4
            lcd_goto(0, 4);
            lcd_print("ELE-3717");
            // Centraliza "FILTRO FIR" (10 chars) -> posição (16-10)/2 = 3
            lcd_goto(1, 3);
            lcd_print("FILTRO FIR");
            
            // Aguarda qualquer botão S1 ou S2 para ir para STATE_BUTTONS
            while(current_state == STATE_INITIAL && !btn_read(BTN_S1) && !btn_read(BTN_S2)) {
                // Verifica S3 novamente dentro do loop
                if (btn_read(BTN_S3) && !last_btn_state) {
                    current_state = STATE_COEFFICIENTS;
                    coef_index = 0;
                    last_btn_state = 1;
                    _delay_ms(300);
                    break;
                }
                last_btn_state = btn_read(BTN_S3);
                _delay_ms(50);
            }
            if (current_state == STATE_INITIAL) {
                current_state = STATE_BUTTONS;
                _delay_ms(300); // Debounce
            }
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
                else {
                    lcd_goto(1, 0);
                    lcd_print("               ");
                }
                
                // Verifica S3 dentro do loop
                uint8_t current_s3 = btn_read(BTN_S3);
                if (current_s3 && !last_btn_state) {
                    current_state = STATE_COEFFICIENTS;
                    coef_index = 0;
                    _delay_ms(300);
                    break;
                }
                last_btn_state = current_s3;
                _delay_ms(50);
            }
            break;
            
        case STATE_COEFFICIENTS:
            lcd_clear();
            // Centraliza "Coeficiente" (11 chars) -> posição (16-11)/2 = 2.5 ≈ 3
            lcd_goto(0, 3);
            lcd_print("Coeficiente");
            lcd_goto(1, 0);
            lcd_print("C");
            lcd_print_dec(coef_index);
            lcd_print(":");
            // Posiciona os asteriscos após "Cx: "
            for (uint8_t i = 0; i < 12; i++) {
                lcd_print("*");
            }
            
            // Aguarda próximo pressionamento de S3 ou outros botões
            while(current_state == STATE_COEFFICIENTS) {
                uint8_t current_s3 = btn_read(BTN_S3);
                if (current_s3 && !last_btn_state) {
                    coef_index++;
                    if (coef_index > 15) {
                        current_state = STATE_INITIAL;
                        coef_index = 0;
                    }
                    last_btn_state = 1;
                    _delay_ms(300);
                    break;
                }
                last_btn_state = current_s3;
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
