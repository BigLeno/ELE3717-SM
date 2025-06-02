#include "fir.h"
#include <avr/io.h>

static fir_filter_t filter;

void fir_init(void) {
    // Inicializa a linha de atraso com zeros
    for (uint8_t i = 0; i < FIR_NUM_TAPS; i++) {
        filter.delay_line[i] = 0;
        filter.coefficients[i] = 128; // Valores padrão (meio da escala)
    }
    filter.index = 0;
    
    // Configura pinos de saída para DAC R2R
    // PC4, PC5 como saída
    DDRC |= (1 << PC4) | (1 << PC5);
    // PB0-PB5 como saída
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5);
    
    // Inicializa saídas em zero
    PORTC &= ~((1 << PC4) | (1 << PC5));
    PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5));
}

void fir_set_coefficients(uint8_t *coeffs) {
    for (uint8_t i = 0; i < FIR_NUM_TAPS; i++) {
        filter.coefficients[i] = coeffs[i];
    }
}

uint8_t fir_process(uint16_t input) {
    // Adiciona nova amostra na linha de atraso
    filter.delay_line[filter.index] = input;
    
    // Calcula saída do filtro FIR
    uint32_t output = 0;
    uint8_t delay_idx = filter.index;
    
    for (uint8_t i = 0; i < FIR_NUM_TAPS; i++) {
        // Multiplica amostra pelo coeficiente
        output += (uint32_t)filter.delay_line[delay_idx] * filter.coefficients[i];
        
        // Decrementa índice da linha de atraso (circular)
        if (delay_idx == 0) {
            delay_idx = FIR_NUM_TAPS - 1;
        } else {
            delay_idx--;
        }
    }
    
    // Avança índice circular
    filter.index++;
    if (filter.index >= FIR_NUM_TAPS) {
        filter.index = 0;
    }
    
    // Normaliza saída para 8 bits
    // Divide por (16 * 255) para normalizar (16 coeficientes * valor máximo do coeficiente)
    // Depois escala para utilizar a entrada de 10 bits
    output = output >> 12; // Divisão aproximada por 4096 (16*256)
    
    if (output > 255) {
        output = 255;
    }
    
    return (uint8_t)output;
}

void fir_output_dac(uint8_t value) {
    // Saída para malha R2R usando PC4, PC5, PB0-PB5
    // PC4 = bit 0 (LSB), PC5 = bit 1, PB0 = bit 2, ..., PB5 = bit 7 (MSB)
    
    // Limpa bits anteriores
    PORTC &= ~((1 << PC4) | (1 << PC5));
    PORTB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2) | (1 << PB3) | (1 << PB4) | (1 << PB5));
    
    // Configura bits conforme o valor
    if (value & 0x01) PORTC |= (1 << PC4); // bit 0
    if (value & 0x02) PORTC |= (1 << PC5); // bit 1
    if (value & 0x04) PORTB |= (1 << PB0); // bit 2
    if (value & 0x08) PORTB |= (1 << PB1); // bit 3
    if (value & 0x10) PORTB |= (1 << PB2); // bit 4
    if (value & 0x20) PORTB |= (1 << PB3); // bit 5
    if (value & 0x40) PORTB |= (1 << PB4); // bit 6
    if (value & 0x80) PORTB |= (1 << PB5); // bit 7
}
