#include <avr/io.h>
#include <util/delay.h>

#include "modulador.h"

static const uint8_t COS_TABLE[32] = {
    255, 253, 246, 234, 218, 199, 177, 153,
    128, 103, 79, 57, 38, 22, 10, 3,
    0, 3, 10, 22, 38, 57, 79, 103,
    128, 153, 177, 199, 218, 234, 246, 253};

uint16_t modula_am(uint16_t msg, uint8_t cont)
{
    int16_t sinal_modulado = 0;
    int8_t portadora = 0;
    uint8_t msg_8bits = msg >> 2;

    portadora = COS_TABLE[cont] - 127;

    sinal_modulado = msg_8bits * portadora;

    sinal_modulado = (sinal_modulado >> 9) + 127; // Ajusta o sinal modulado para 8 bits

    return sinal_modulado;
}

uint16_t modula_fm(uint16_t msg, uint8_t cont)
{
    int8_t portadora = 0;

    OCR1A = 600 + (1023 - msg) * 2;
    portadora = COS_TABLE[cont];

    return portadora;
}

uint8_t modula_ask(uint16_t msg, uint8_t cont)
{
    int8_t portadora = 0;

    portadora = msg * COS_TABLE[cont];

    return portadora;
}
uint8_t modula_fsk(uint16_t msg, uint8_t cont)
{
    int8_t portadora = 0;

    OCR1A = 600 + 16000 * msg;
    portadora = COS_TABLE[cont];

    return portadora;
}

void envia_dados(uint16_t y)
{
    PORTC = (y << 4) & 0x30; // Envia os bits menos significativos para PC5 e PC6 0000 00xx
    PORTB = (y >> 2);        // Envia os bits mais significativos para PB0 a PB5  xxxx xx00
}