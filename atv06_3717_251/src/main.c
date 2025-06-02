#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "lcd.h"
#include "btn.h"
#include "mde.h"
#include "adc.h"
#include "fir.h"

// PC0 - Input para o filtro FIR
// PC4 - Output para o filtro FIR (LSB)
// PC5 - Output para o filtro FIR 
// PB0 - Output para o filtro FIR 
// PB1 - Output para o filtro FIR 
// PB2 - Output para o filtro FIR
// PB3 - Output para o filtro FIR 
// PB4 - Output para o filtro FIR
// PB5 - Output para o filtro FIR (MSB)

// --- Função para mostrar bits como string com espaços ---
void bits_to_string(uint8_t val, char* str) {
    for (uint8_t i = 0; i < 8; i++) {
        str[i * 2] = (val & (1 << (7 - i))) ? '1' : '0';
        str[i * 2 + 1] = ' ';
    }
    str[16] = '\0';
}

// --- Main ---

int main() {
    lcd_init();
    btn_init();
    adc_init();
    fir_init();
    mde_init();
    
    while(1) {
        mde_run();
        
        // Processa filtro FIR continuamente
        uint16_t adc_value = adc_read_a0();
        uint8_t filtered_output = fir_process(adc_value);
        fir_output_dac(filtered_output);
        
        _delay_us(100); // Pequeno delay para controlar taxa de amostragem
    }
}