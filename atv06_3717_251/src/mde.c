#include "mde.h"
#include "lcd.h"
#include "btn.h"
#include "eeprom.h"
#include "fir.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

static state_t current_state = STATE_INITIAL;
static uint8_t coef_index = 0;
static float coefficients[NUM_COEFFICIENTS] = {0.0f}; // Array de coeficientes float

// Declarações extern para as flags definidas em main.c
extern volatile uint8_t flag_btn_s1;
extern volatile uint8_t flag_btn_s2;
extern volatile uint8_t flag_btn_s3;

void mde_init(void) {
    current_state = STATE_INITIAL;
    coef_index = 0;
    // Inicializa com coeficientes do filtro FIR passa-baixa calculado no Python
    mde_load_default_coefficients();
}

void mde_load_default_coefficients(void) {
    // Coeficientes calculados no Python com scipy.signal.firwin()
    // fc=2Hz, fs=100Hz, 16 taps, arredondados para 3 casas decimais
    // Soma total: 1.000000 para normalização correta
    coefficients[0] = 0.009f;   // 0.009000
    coefficients[1] = 0.013f;   // 0.013000
    coefficients[2] = 0.027f;   // 0.027000
    coefficients[3] = 0.047f;   // 0.047000
    coefficients[4] = 0.071f;   // 0.071000
    coefficients[5] = 0.095f;   // 0.095000
    coefficients[6] = 0.114f;   // 0.114000
    coefficients[7] = 0.124f;   // 0.124000
    coefficients[8] = 0.124f;   // 0.124000
    coefficients[9] = 0.114f;   // 0.114000
    coefficients[10] = 0.095f;  // 0.095000
    coefficients[11] = 0.071f;  // 0.071000
    coefficients[12] = 0.047f;  // 0.047000
    coefficients[13] = 0.027f;  // 0.027000
    coefficients[14] = 0.013f;  // 0.013000
    coefficients[15] = 0.009f;  // 0.009000
    
    mde_update_filter();
}

void mde_update_filter(void) {
    // Atualiza os coeficientes do filtro FIR
    fir_set_coefficients(coefficients);
}

// Funções simplificadas para demo - coeficientes float não são facilmente editáveis via botões
void mde_save_coefficients(void) {
    // Para implementação futura: salvar coeficientes float na EEPROM
    // Requer conversão para formato adequado de armazenamento
}

void mde_load_coefficients(void) {
    // Para implementação futura: carregar coeficientes float da EEPROM
    // Por enquanto, usa valores padrão
    mde_load_default_coefficients();
}

void mde_run(void) {
    // Serviço de tela em background
    switch(current_state) {
        case STATE_INITIAL:
            lcd_clear();
            lcd_goto(0, 4);
            lcd_print("ELE-3717");
            lcd_goto(1, 1);
            lcd_print("FILTRO FIR 2Hz");
            // Aguarda qualquer botão para sair da tela inicial
            while(current_state == STATE_INITIAL) {
                if (flag_btn_s3) {
                    current_state = STATE_COEFFICIENTS;
                    coef_index = 0;
                    flag_btn_s3 = 0;
                    _delay_ms(300);
                    break;
                }
                if (flag_btn_s1 || flag_btn_s2) {
                    current_state = STATE_BUTTONS;
                    flag_btn_s1 = 0;
                    flag_btn_s2 = 0;
                    _delay_ms(300);
                    break;
                }
            }
            break;
        case STATE_BUTTONS:
            lcd_clear();
            // Centraliza "Press buttons:" (14 chars) -> posição (16-14)/2 = 1
            lcd_goto(0, 1);
            lcd_print("Press buttons:");
            while(current_state == STATE_BUTTONS) {
                if (flag_btn_s1) {
                    lcd_goto(1, 0);
                    lcd_print("S1 pressed     ");
                    flag_btn_s1 = 0;
                    _delay_ms(200);
                }
                else if (flag_btn_s2) {
                    lcd_goto(1, 0);
                    lcd_print("S2 pressed     ");
                    flag_btn_s2 = 0;
                    _delay_ms(200);
                }
                else {
                    lcd_goto(1, 0);
                    lcd_print("               ");
                }
                if (flag_btn_s3) {
                    current_state = STATE_COEFFICIENTS;
                    coef_index = 0;
                    flag_btn_s3 = 0;
                    _delay_ms(300);
                    break;
                }
            }
            break;
        case STATE_COEFFICIENTS:
            lcd_clear();
            lcd_goto(0, 2);
            lcd_print("Coef Float");
            lcd_goto(1, 0);
            lcd_print("C");
            if (coef_index < 10) lcd_print("0");
            lcd_print_dec(coef_index);
            lcd_print(": ");
            
            // Exibe coeficiente float com 3 casas decimais
            // Converte para inteiro multiplicado por 1000 para exibição
            int16_t coef_display = (int16_t)(coefficients[coef_index] * 1000.0f);
            if (coef_display < 0) {
                lcd_print("-");
                coef_display = -coef_display;
            }
            lcd_print_dec(coef_display / 1000);
            lcd_print(".");
            lcd_print_dec((coef_display % 1000) / 100);
            lcd_print_dec((coef_display % 100) / 10);
            lcd_print_dec(coef_display % 10);
            
            while(current_state == STATE_COEFFICIENTS) {
                if (flag_btn_s3) {
                    coef_index++;
                    if (coef_index > 15) {
                        current_state = STATE_INITIAL;
                        coef_index = 0;
                    }
                    flag_btn_s3 = 0;
                    _delay_ms(300);
                    break;
                }
                // S1 e S2 apenas para navegação - edição de float é complexa
                if (flag_btn_s1 || flag_btn_s2) {
                    flag_btn_s1 = 0;
                    flag_btn_s2 = 0;
                    _delay_ms(150);
                }
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
