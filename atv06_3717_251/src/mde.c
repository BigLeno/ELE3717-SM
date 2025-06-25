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
    // Coeficientes passa-baixa fs=100Hz, fc=2Hz, 16 taps, 3 casas decimais
    coefficients[0]  = 0.010f;
    coefficients[1]  = 0.015f;
    coefficients[2]  = 0.028f;
    coefficients[3]  = 0.049f;
    coefficients[4]  = 0.072f;
    coefficients[5]  = 0.094f;
    coefficients[6]  = 0.112f;
    coefficients[7]  = 0.121f;
    coefficients[8]  = 0.121f;
    coefficients[9]  = 0.112f;
    coefficients[10] = 0.094f;
    coefficients[11] = 0.072f;
    coefficients[12] = 0.049f;
    coefficients[13] = 0.028f;
    coefficients[14] = 0.015f;
    coefficients[15] = 0.010f;

    mde_update_filter();
}

void mde_update_filter(void) {
    fir_set_coefficients(coefficients);
}

void mde_save_coefficients(void) {
    // Salva os coeficientes float na EEPROM, cada float ocupa 4 bytes
    // Salva a partir do endereço 0
    uint16_t addr = 0;
    for (uint8_t i = 0; i < NUM_COEFFICIENTS; i++) {
        float value = coefficients[i];
        uint8_t *p = (uint8_t*)&value;
        for (uint8_t b = 0; b < 4; b++) {
            eeprom_write_byte(addr++, p[b]);
        }
    }
}

void mde_load_coefficients(void) {
    mde_load_default_coefficients();
}

void mde_run(void) {
    // Máquina de estados NÃO BLOQUEANTE: apenas atualiza tela/estado, nunca entra em laço while interno
    static state_t last_state = 0xFF;

    if (current_state != last_state) {
        switch(current_state) {
            case STATE_INITIAL:
                lcd_clear();
                lcd_goto(0, 4);
                lcd_print("ELE-3717");
                lcd_goto(1, 1);
                lcd_print("FILTRO FIR 2Hz");
                break;
            case STATE_COEFFICIENTS:
                lcd_clear();
                lcd_goto(0, 2);
                lcd_print("Coef Float");
                break;
        }
        last_state = current_state;
    }

    switch(current_state) {
        case STATE_INITIAL:
            if (flag_btn_s3) {
                current_state = STATE_COEFFICIENTS;
                coef_index = 0;
                flag_btn_s3 = 0;
                _delay_ms(300);
            }
            // S1 e S2 não fazem nada fora do estado coeficientes
            flag_btn_s1 = 0;
            flag_btn_s2 = 0;
            break;
        case STATE_COEFFICIENTS:
            lcd_goto(1, 0);
            lcd_print("C");
            if (coef_index < 10) lcd_print("0");
            lcd_print_dec(coef_index);
            lcd_print(": ");
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
            lcd_print("   ");

            if (flag_btn_s3) {
                coef_index++;
                if (coef_index > 15) {
                    current_state = STATE_INITIAL;
                    coef_index = 0;
                }
                flag_btn_s3 = 0;
                _delay_ms(300);
            }
            if (flag_btn_s1 || flag_btn_s2) {
                flag_btn_s1 = 0;
                flag_btn_s2 = 0;
                _delay_ms(150);
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
