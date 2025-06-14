#include "mde.h"
#include "lcd.h"
#include "btn.h"
#include "eeprom.h"
#include "fir.h"
#include <avr/io.h>
#include <util/delay.h>

static state_t current_state = STATE_INITIAL;
static uint8_t coef_index = 0;
static uint8_t last_btn_s3_state = 0;
static uint8_t last_btn_s2_state = 0;
static uint8_t last_btn_s1_state = 0;
static uint8_t coefficients[NUM_COEFFICIENTS] = {0}; // Array de coeficientes inicializados com 0

void mde_init(void) {
    current_state = STATE_INITIAL;
    coef_index = 0;
    // Carrega coeficientes da EEPROM
    mde_load_coefficients();
}

void mde_update_filter(void) {
    // Atualiza os coeficientes do filtro FIR
    fir_set_coefficients(coefficients);
}

void mde_save_coefficients(void) {
    eeprom_save_coefficients(coefficients, NUM_COEFFICIENTS);
}

void mde_load_coefficients(void) {
    eeprom_load_coefficients(coefficients, NUM_COEFFICIENTS);
    
    // Verifica se há valores válidos na EEPROM (primeira inicialização)
    // Se todos os coeficientes forem 0xFF (EEPROM virgem), inicializa com valores de filtro passa-baixa
    uint8_t all_empty = 1;
    for (uint8_t i = 0; i < NUM_COEFFICIENTS; i++) {
        if (coefficients[i] != 0xFF) {
            all_empty = 0;
            break;
        }
    }
    
    if (all_empty) {
        // Inicializa com coeficientes de filtro passa-baixa
        // Coeficientes decrescentes para criar resposta passa-baixa
        coefficients[0] = 255;   // Coeficiente mais alto
        coefficients[1] = 240;
        coefficients[2] = 220;
        coefficients[3] = 195;
        coefficients[4] = 165;
        coefficients[5] = 135;
        coefficients[6] = 105;
        coefficients[7] = 80;
        coefficients[8] = 60;
        coefficients[9] = 45;
        coefficients[10] = 35;
        coefficients[11] = 25;
        coefficients[12] = 20;
        coefficients[13] = 15;
        coefficients[14] = 10;
        coefficients[15] = 5;    // Coeficiente mais baixo
        
        mde_save_coefficients();
    }
}

void mde_run(void) {
    uint8_t btn_s3_pressed = btn_read(BTN_S3);
    
    // Detecta borda de subida do botão S3 (Menu)
    if (btn_s3_pressed && !last_btn_s3_state) {
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
    last_btn_s3_state = btn_s3_pressed;
    
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
                if (btn_read(BTN_S3) && !last_btn_s3_state) {
                    current_state = STATE_COEFFICIENTS;
                    coef_index = 0;
                    last_btn_s3_state = 1;
                    _delay_ms(300);
                    break;
                }
                last_btn_s3_state = btn_read(BTN_S3);
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
                if (current_s3 && !last_btn_s3_state) {
                    current_state = STATE_COEFFICIENTS;
                    coef_index = 0;
                    _delay_ms(300);
                    break;
                }
                last_btn_s3_state = current_s3;
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
            if (coef_index < 10) {
                lcd_print("0");
            }
            lcd_print_dec(coef_index);
            lcd_print(": ");
            lcd_print_dec(coefficients[coef_index]);
            
            // Aguarda botões para ajustar coeficiente ou navegar
            while(current_state == STATE_COEFFICIENTS) {
                uint8_t current_s3 = btn_read(BTN_S3);
                uint8_t current_s2 = btn_read(BTN_S2);
                uint8_t current_s1 = btn_read(BTN_S1);
                
                // Botão S3 (Menu) - próximo coeficiente ou volta ao inicial
                if (current_s3 && !last_btn_s3_state) {
                    coef_index++;
                    if (coef_index > 15) {
                        current_state = STATE_INITIAL;
                        coef_index = 0;
                    }
                    last_btn_s3_state = 1;
                    _delay_ms(300);
                    break;
                }
                
                // Botão S2 (▲) - incrementa coeficiente
                if (current_s2 && !last_btn_s2_state) {
                    if (coefficients[coef_index] < COEF_MAX) {
                        coefficients[coef_index]++;
                        mde_update_filter(); // Atualiza o filtro
                        eeprom_save_coefficient(coef_index, coefficients[coef_index]); // Salva na EEPROM
                        
                        // Atualiza display imediatamente
                        lcd_goto(1, 5);
                        lcd_print("   "); // Limpa valor antigo
                        lcd_goto(1, 5);
                        lcd_print_dec(coefficients[coef_index]);
                    }
                    _delay_ms(150); // Debounce menor para repetição
                }
                
                // Botão S1 (▼) - decrementa coeficiente
                if (current_s1 && !last_btn_s1_state) {
                    if (coefficients[coef_index] > COEF_MIN) {
                        coefficients[coef_index]--;
                        mde_update_filter(); // Atualiza o filtro
                        eeprom_save_coefficient(coef_index, coefficients[coef_index]); // Salva na EEPROM
                        
                        // Atualiza display imediatamente
                        lcd_goto(1, 5);
                        lcd_print("   "); // Limpa valor antigo
                        lcd_goto(1, 5);
                        lcd_print_dec(coefficients[coef_index]);
                    }
                    _delay_ms(150); // Debounce menor para repetição
                }
                
                last_btn_s3_state = current_s3;
                last_btn_s2_state = current_s2;
                last_btn_s1_state = current_s1;
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
