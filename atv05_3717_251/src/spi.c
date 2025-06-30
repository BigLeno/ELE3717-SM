#include <avr/io.h>
#include "spi.h"

#define DATA_PORT_PORT PORTB
#define DATA_PORT_DDR  DDRB
#define DIN PB3     // SPI MOSI
#define CS  PB2     // Chip select
#define CLK PB5     // SPI Clock

void spi_init(void) {
    DATA_PORT_DDR |= (1 << DIN) | (1 << CS) | (1 << CLK);
    SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0);
    DATA_PORT_PORT |= (1 << CS);
}

void spi_transmit(uint8_t data) {
    SPDR = data;
    while (!(SPSR & (1 << SPIF)));
}