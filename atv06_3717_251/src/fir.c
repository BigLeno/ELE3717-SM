#include "fir.h"
#include <avr/io.h>

static fir_filter_t filter;

void fir_init(void) {
    // Inicializa a linha de atraso com zeros
    for (uint8_t i = 0; i < FIR_NUM_TAPS; i++) {
        filter.delay_line[i] = 0;
    }
    
    // Inicializa coeficientes com valores de filtro passa-baixa
    filter.coefficients[0] = 255;
    filter.coefficients[1] = 240;
    filter.coefficients[2] = 220;
    filter.coefficients[3] = 195;
    filter.coefficients[4] = 165;
    filter.coefficients[5] = 135;
    filter.coefficients[6] = 105;
    filter.coefficients[7] = 80;
    filter.coefficients[8] = 60;
    filter.coefficients[9] = 45;
    filter.coefficients[10] = 35;
    filter.coefficients[11] = 25;
    filter.coefficients[12] = 20;
    filter.coefficients[13] = 15;
    filter.coefficients[14] = 10;
    filter.coefficients[15] = 5;
    
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
    
    // Normalização melhorada para máximo aproveitamento da faixa dinâmica
    // Máximo teórico: 1023 × 255 × 16 = 4,173,840
    // Para usar toda faixa 0-255: dividir por (4,173,840 / 255) = 16,368
    // Aproximação: shift 14 (÷16,384) está próximo e é eficiente
    output = output >> 14;
    
    // Proteção contra overflow (redundante, mas garante segurança)
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
