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

void ler_adc(uint16_t *adc_value)
{
    ADCSRA |= (1 << ADSC); // inicia a conversao
    while (!(ADCSRA & (1 << ADIF))); // espera a conversao terminar

    ADCSRA |= (1 << ADIF); // limpa a flag de interrupcao

    *adc_value = ADC; // armazena o valor lido
    _delay_us(10);
}