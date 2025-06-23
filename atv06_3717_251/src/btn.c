#include "btn.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Flags globais dos botões
volatile uint8_t flag_btn_s1 = 0;
volatile uint8_t flag_btn_s2 = 0;
volatile uint8_t flag_btn_s3 = 0;

// Debounce robusto: só seta a flag se o botão estava solto antes e agora está pressionado
static uint8_t last_state_s1 = 1;
static uint8_t last_state_s2 = 1;
static uint8_t last_state_s3 = 1;

ISR(PCINT1_vect) {
    // S1: PC1
    uint8_t curr_s1 = (PINC & (1 << BTN_S1)) ? 1 : 0;
    if (last_state_s1 && !curr_s1) {
        flag_btn_s1 = 1;
    }
    last_state_s1 = curr_s1;

    // S2: PC2
    uint8_t curr_s2 = (PINC & (1 << BTN_S2)) ? 1 : 0;
    if (last_state_s2 && !curr_s2) {
        flag_btn_s2 = 1;
    }
    last_state_s2 = curr_s2;

    // S3: PC3
    uint8_t curr_s3 = (PINC & (1 << BTN_S3)) ? 1 : 0;
    if (last_state_s3 && !curr_s3) {
        flag_btn_s3 = 1;
    }
    last_state_s3 = curr_s3;
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
