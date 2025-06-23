#include "btn.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Flags globais dos botões
volatile uint8_t flag_btn_s1 = 0;
volatile uint8_t flag_btn_s2 = 0;
volatile uint8_t flag_btn_s3 = 0;

// ISR dos botões (PCINT1_vect para PC1, PC2, PC3)
ISR(PCINT1_vect) {
    // S1: PC1
    if (!(PINC & (1 << BTN_S1))) {
        flag_btn_s1 = 1;
        _delay_ms(20); // debounce simples (bloqueante, mas seguro)
    }
    // S2: PC2
    if (!(PINC & (1 << BTN_S2))) {
        flag_btn_s2 = 1;
        _delay_ms(20);
    }
    // S3: PC3
    if (!(PINC & (1 << BTN_S3))) {
        flag_btn_s3 = 1;
        _delay_ms(20);
    }
}

void btn_init(void) {
    // Configura PC1, PC2, PC3 como entrada
    DDRC &= ~((1 << BTN_S1) | (1 << BTN_S2) | (1 << BTN_S3));
    
    // Habilita pull-up interno
    PORTC |= (1 << BTN_S1) | (1 << BTN_S2) | (1 << BTN_S3);
}

uint8_t btn_read(uint8_t btn) {
    // Retorna 1 se botão pressionado (lógica invertida devido ao pull-up)
    return !(PINC & (1 << btn));
}
