#include "adc.h"
#include <avr/io.h>

void adc_init(void) {
    // ADMUX = 01000000b
    // Bit 7-6: REFS = 01 → AVCC como referência (5V)
    // Bit 5: ADLAR = 0 → Resultado justificado à direita
    // Bit 4: Reservado
    // Bit 3-0: MUX = 0000 → Canal ADC0 (PC0)
    ADMUX = (1 << REFS0);
    
    // ADCSRA = 10000111b
    // Bit 7: ADEN = 1 → Habilita ADC
    // Bit 6: ADSC = 0 → Não inicia conversão ainda
    // Bit 5: ADATE = 0 → Modo single conversion
    // Bit 4: ADIF = 0 → Flag de interrupção
    // Bit 3: ADIE = 0 → Interrupção desabilitada
    // Bit 2-0: ADPS = 111 → Prescaler 128
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    // Configura PC0 como entrada de alta impedância
    DDRC &= ~(1 << PC0);    // Entrada (tri-state)
    PORTC &= ~(1 << PC0);   // Pull-up desabilitado
}

uint16_t adc_read(uint8_t channel) {
    // Seleciona o canal (0-7)
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F);
    
    // Inicia conversão
    ADCSRA |= (1 << ADSC);
    
    // Aguarda conversão terminar
    while (ADCSRA & (1 << ADSC));
    
    // Retorna resultado de 10 bits
    return ADC;
}

uint16_t adc_read_a0(void) {
    return adc_read(ADC_CHANNEL_A0);
}
