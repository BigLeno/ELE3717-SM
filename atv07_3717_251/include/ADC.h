#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

void ADC_init(void);
void ler_adc(uint16_t *adc_value);

#endif