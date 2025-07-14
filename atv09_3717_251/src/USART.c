#include "USART.h"
#include <stdlib.h>  // Para itoa()

void USART_init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    UCSR0B = (1 << RXEN0) | (1 << TXEN0);
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void USART_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0)));
    UDR0 = data;
}

unsigned char USART_receive(void) {
    while (!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void USART_send_string(const char* str) {
    while (*str) {
        USART_transmit(*str++);
    }
}

void USART_send_int(int value) {
    char buffer[10]; // Buffer para armazenar o inteiro convertido (suficiente para -32768 a 32767)
    itoa(value, buffer, 10); // Converte inteiro para string decimal
    USART_send_string(buffer);
}

void USART_send_float(float value, uint8_t decimals) {
    // Converte float para inteiro multiplicando por 10^decimals
    int multiplier = 1;
    for (uint8_t i = 0; i < decimals; i++) {
        multiplier *= 10;
    }
    
    int int_part = (int)value;
    int frac_part = (int)((value - int_part) * multiplier);
    
    // Trata números negativos
    if (value < 0 && int_part == 0) {
        USART_transmit('-');
    }
    
    USART_send_int(int_part);
    USART_transmit('.');
    
    // Adiciona zeros à esquerda se necessário
    int temp_mult = multiplier / 10;
    while (temp_mult > frac_part && temp_mult > 1) {
        USART_transmit('0');
        temp_mult /= 10;
    }
    
    if (frac_part < 0) frac_part = -frac_part; // Remove sinal negativo da parte fracionária
    USART_send_int(frac_part);
}