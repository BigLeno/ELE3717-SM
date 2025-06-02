#include "btn.h"
#include <avr/io.h>

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
