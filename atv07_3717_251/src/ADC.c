#include "ADC.h"
#include <avr/io.h>
#include <util/delay.h>

void ADC_init(void)
{
    ADMUX = 0x00;
    ADMUX |= 0x40; // configura ADC, ADC0 como leitura
    ADCSRA = 0x00;
    ADCSRA |= 0x87;
    ADCSRB = 0x00;
}

uint16_t ler_adc()
{
    ADCSRA |= (1 << ADSC); // inicia a conversao
    while (!(ADCSRA & (1 << ADIF))); // espera a conversao terminar

    return ADC; // armazena o valor lido
}