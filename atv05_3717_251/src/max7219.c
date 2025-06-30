#include "max7219.h"
#include "spi.h"
#include <avr/io.h>

#define DATA_PORT_PORT PORTB
#define CS PB2

void max7219_send(uint8_t addr, uint8_t data) {
    DATA_PORT_PORT &= ~(1 << CS);   // CS ativo (LOW)
    spi_transmit(addr);
    spi_transmit(data);
    DATA_PORT_PORT |= (1 << CS);    // CS inativo (HIGH)
}

void max7219_init(void) {
    max7219_send(0x0F, 0x00); // Display test off
    max7219_send(0x09, 0x00); // Decode mode off
    max7219_send(0x0A, 0x03); // Intensidade nível 3
    max7219_send(0x0B, 0x07); // Scan limit 8 digits
    max7219_send(0x0C, 0x01); // Normal operation
}

void max7219_clear(void) {
    for (uint8_t i = 1; i <= 8; i++) {
        max7219_send(i, 0x00);
    }
}