#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "lcd.h"
#include "btn.h"
#include "mde.h"
#include "adc.h"
#include "fir.h"

/*
 * FILTRO FIR PASSA-BAIXA DIGITAL - CARACTERÍSTICAS:
 * 
 * ENTRADA:
 * - PC0 (A0): Sinal analógico 0-5V, resolução 10 bits (0-1023)
 * 
 * PROCESSAMENTO:
 * - Filtro FIR de 16 coeficientes (ordem 15)
 * - Coeficientes de 8 bits (0-255) para ajuste fino
 * - Algoritmo de convolução digital em tempo real
 * 
 * SAÍDA:
 * - PC4 (LSB): bit 0 do DAC R2R
 * - PC5: bit 1 do DAC R2R
 * - PB0-PB5: bits 2-7 do DAC R2R (PB5 = MSB)
 * - Resolução: 8 bits (0-255)
 * 
 * DESEMPENHO DO FILTRO:
 * - Taxa de amostragem: ~10 kHz (delay de 100μs)
 * - Frequência de corte: ~1.25 kHz (-3dB)
 * - Atenuação: -40dB/década após frequência de corte
 * - Resposta: Passa-baixa com rejeição de ruído de alta frequência
 * - Estabilidade: Sempre estável (característica FIR)
 * 
 * CONFIGURAÇÃO:
 * - Coeficientes ajustáveis via interface LCD e botões
 * - Salvamento automático na EEPROM
 * - Atualização em tempo real do filtro
 */

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
        // Taxa de amostragem: ~10 kHz (100μs de período)
        uint16_t adc_value = adc_read_a0();
        uint8_t filtered_output = fir_process(adc_value);
        fir_output_dac(filtered_output);
        
        _delay_us(100); // Controla taxa de amostragem: 10 kHz
    }
}