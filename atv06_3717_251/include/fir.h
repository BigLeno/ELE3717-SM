#ifndef FIR_H
#define FIR_H

#include <stdint.h>

// Configurações do filtro FIR
#define FIR_NUM_TAPS 16
#define FIR_INPUT_BITS 10
#define FIR_OUTPUT_BITS 8

// Estrutura do filtro FIR
typedef struct {
    uint16_t delay_line[FIR_NUM_TAPS];  // Linha de atraso para amostras
    uint8_t coefficients[FIR_NUM_TAPS]; // Coeficientes do filtro
    uint8_t index;                      // Índice circular da linha de atraso
} fir_filter_t;

void fir_init(void);
void fir_set_coefficients(uint8_t *coeffs);
uint8_t fir_process(uint16_t input);
void fir_output_dac(uint8_t value);

#endif // FIR_H
