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

// Flags de botões (definidas como extern em mde.c)
volatile uint8_t flag_btn_s1 = 0;
volatile uint8_t flag_btn_s2 = 0;
volatile uint8_t flag_btn_s3 = 0;

// ISRs dos botões
ISR(INT0_vect) { flag_btn_s1 = 1; }
ISR(INT1_vect) { flag_btn_s2 = 1; }
ISR(PCINT2_vect) {
    if (!(PIND & (1 << PD4))) flag_btn_s3 = 1;
}

void setup_interrupts(void) {
    // INT0 (PD2) - S1
    EICRA |= (1 << ISC01);
    EIMSK |= (1 << INT0);
    // INT1 (PD3) - S2
    EICRA |= (1 << ISC11);
    EIMSK |= (1 << INT1);
    // PCINT20 (PD4) - S3
    PCICR |= (1 << PCIE2);
    PCMSK2 |= (1 << PCINT20);
    PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4); // Pull-ups
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