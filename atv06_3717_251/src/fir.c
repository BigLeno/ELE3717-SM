#include "fir.h"
#include <avr/io.h>

static fir_filter_t filter;

void fir_init(void) {
    // Inicializa a linha de atraso com zeros
    for (uint8_t i = 0; i < FIR_NUM_TAPS; i++) {
        filter.delay_line[i] = 0;
    }
    
    // Coeficientes do filtro FIR passa-baixa (fc=2Hz, fs=100Hz) com 3 casas decimais
    // Calculados com scipy.signal.firwin() e arredondados conforme análise Python
    filter.coefficients[0] = 0.009f;   // 0.009000
    filter.coefficients[1] = 0.013f;   // 0.013000
    filter.coefficients[2] = 0.027f;   // 0.027000
    filter.coefficients[3] = 0.047f;   // 0.047000
    filter.coefficients[4] = 0.071f;   // 0.071000
    filter.coefficients[5] = 0.095f;   // 0.095000
    filter.coefficients[6] = 0.114f;   // 0.114000
    filter.coefficients[7] = 0.124f;   // 0.124000
    filter.coefficients[8] = 0.124f;   // 0.124000
    filter.coefficients[9] = 0.114f;   // 0.114000
    filter.coefficients[10] = 0.095f;  // 0.095000
    filter.coefficients[11] = 0.071f;  // 0.071000
    filter.coefficients[12] = 0.047f;  // 0.047000
    filter.coefficients[13] = 0.027f;  // 0.027000
    filter.coefficients[14] = 0.013f;  // 0.013000
    filter.coefficients[15] = 0.009f;  // 0.009000
    
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

void fir_set_coefficients(float *coeffs) {
    for (uint8_t i = 0; i < FIR_NUM_TAPS; i++) {
        filter.coefficients[i] = coeffs[i];
    }
}

uint8_t fir_process(uint16_t input) {
    // Adiciona nova amostra na linha de atraso
    filter.delay_line[filter.index] = input;
    
    // Calcula saída do filtro FIR usando aritmética float
    float output = 0.0f;
    uint8_t delay_idx = filter.index;
    
    for (uint8_t i = 0; i < FIR_NUM_TAPS; i++) {
        // Multiplica amostra pelo coeficiente (float)
        output += (float)filter.delay_line[delay_idx] * filter.coefficients[i];
        
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
    
    // Normalização para saída 8 bits (0-255)
    // Como a soma dos coeficientes é aproximadamente 1.0, 
    // o resultado já está na faixa adequada para ADC de 10 bits
    // Mapeia de 0-1023 para 0-255
    output = output * 255.0f / 1023.0f;
    
    // Proteção contra overflow/underflow
    if (output > 255.0f) {
        output = 255.0f;
    } else if (output < 0.0f) {
        output = 0.0f;
    }
    
    return (uint8_t)(output + 0.5f); // Arredondamento
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
