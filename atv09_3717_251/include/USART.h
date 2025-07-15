#ifndef USART_H
#define USART_H

#include <avr/io.h>

#define BAUD 9600
#define MYUBRR ((F_CPU / (16UL * BAUD)) - 1)

// Para comunicação com HC-05:
// TX do Arduino (PD1) → RX do HC-05 (com divisor de tensão)
// RX do Arduino (PD0) ← TX do HC-05
// Baud rate padrão do HC-05: 9600
// Funções abaixo enviam dados via TX do Arduino para o Bluetooth

// Inicializa a USART com o valor do registrador UBRR
void USART_init(unsigned int ubrr);

// Transmite um byte pela USART
void USART_transmit(unsigned char data);

// Recebe um byte pela USART
unsigned char USART_receive(void);

// Envia uma string pela USART
void USART_send_string(const char* str);

// Envia um número inteiro como texto
void USART_send_int(int value);

// Envia um número float como texto
void USART_send_float(float value, uint8_t decimals);

#endif