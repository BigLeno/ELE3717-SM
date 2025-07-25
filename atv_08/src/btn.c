#include "btn.h"
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#ifndef BTN_S0
#define BTN_S0 0 // PC0
#endif

// Flags globais dos botões (hardware)
volatile uint8_t flag_btn_s1 = 0;
volatile uint8_t flag_btn_s2 = 0;
volatile uint8_t flag_btn_s3 = 0;
// Flags globais para integração com main.c
volatile uint8_t btn_m_flag = 0;
volatile uint8_t btn_up_flag = 0;
volatile uint8_t btn_down_flag = 0;
volatile uint8_t btn_a_flag = 0;

// Debounce robusto: só seta a flag se o botão estava solto antes e agora está pressionado
static uint8_t last_state_s1 = 1;
static uint8_t last_state_s2 = 1;
static uint8_t last_state_s3 = 1;

ISR(PCINT1_vect) {
    // S0: PC0 (agora faz o que S2 fazia: DOWN)
    static uint8_t last_state_s0 = 1;
    uint8_t curr_s0 = (PINC & (1 << BTN_S0)) ? 1 : 0;
    if (last_state_s0 && !curr_s0) {
        btn_down_flag = 1;
    }
    last_state_s0 = curr_s0;

    // S1: PC1 (agora faz o que S3 fazia: M)
    uint8_t curr_s1 = (PINC & (1 << BTN_S1)) ? 1 : 0;
    if (last_state_s1 && !curr_s1) {
        btn_m_flag = 1;
    }
    last_state_s1 = curr_s1;

    // S2: PC2 (agora faz o que S1 fazia: UP)
    uint8_t curr_s2 = (PINC & (1 << BTN_S2)) ? 1 : 0;
    if (last_state_s2 && !curr_s2) {
        btn_up_flag = 1;
    }
    last_state_s2 = curr_s2;

    // S3: PC3 (agora faz o que S0 fazia: A)
    uint8_t curr_s3 = (PINC & (1 << BTN_S3)) ? 1 : 0;
    if (last_state_s3 && !curr_s3) {
        btn_a_flag = 1;
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
