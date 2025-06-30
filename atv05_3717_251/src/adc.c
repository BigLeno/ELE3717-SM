#include <avr/io.h>
#include "adc.h"

void adc_init(void) {
    ADMUX = (1 << REFS0); // referência AVcc (5V)
    ADCSRA = (1 << ADEN)    // habilita ADC
           | (1 << ADPS2)   // prescaler 128 (ideal para 16 MHz)
           | (1 << ADPS1)
           | (1 << ADPS0);
    DIDR0 = (1 << ADC4D) | (1 << ADC5D); // desliga entradas digitais ADC4, ADC5 para reduzir ruído
}

uint16_t adc_read(uint8_t channel) {
    ADMUX = (ADMUX & 0xF0) | (channel & 0x0F); // seleciona canal ADC0..ADC7
    ADCSRA |= (1 << ADSC);                      // inicia conversão
    while (ADCSRA & (1 << ADSC));               // espera conclusão
    return ADCW;
}

uint8_t adc_get_pos(uint8_t channel) {
    uint16_t val = adc_read(channel);
    uint8_t pos = (val * 8) / 1024;
    if (pos > 7) pos = 7;
    if (pos == 0) pos = 1; // conforme solicitado, evita zero
    return pos;
}