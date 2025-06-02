#include "adc.h"
#include <avr/io.h>

void adc_init(void) {
    // Configura referência como AVCC (5V)
    ADMUX = (1 << REFS0);
    
    // Habilita ADC e configura prescaler para 128 (125kHz @ 16MHz)
    ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
    
    // Configura PC0 (A0) como entrada analógica
    DDRC &= ~(1 << PC0);
    PORTC &= ~(1 << PC0); // Desabilita pull-up
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
