#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "lcd.h"
#include "btn.h"
#include "mde.h"
#include "adc.h"
#include "fir.h"

void setup_interrupts(void) {
    // PCINT9 (PC1) - S1
    // PCINT10 (PC2) - S2
    // PCINT11 (PC3) - S3
    PCICR |= (1 << PCIE1); // Habilita interrupção do grupo PCINT[14:8] (PORTC)
    PCMSK1 |= (1 << PCINT9) | (1 << PCINT10) | (1 << PCINT11);
    // Pull-ups para os botões
    PORTC |= (1 << PC1) | (1 << PC2) | (1 << PC3);
}

int main() {
    lcd_init();
    btn_init();
    adc_init();
    fir_init();
    mde_init();
    setup_interrupts();
    sei();

    while(1) {
        mde_run();
        uint16_t adc_value = adc_read_a0();
        uint8_t filtered_output = fir_process(adc_value);
        fir_output_dac(filtered_output);
        _delay_us(100);
    }
}