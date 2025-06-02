#ifndef ADC_H
#define ADC_H

#include <stdint.h>

// Configurações do ADC
#define ADC_CHANNEL_A0 0

void adc_init(void);
uint16_t adc_read(uint8_t channel);
uint16_t adc_read_a0(void);

#endif // ADC_H
