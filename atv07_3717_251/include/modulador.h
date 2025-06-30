#ifndef MODULADOR_H
#define MODULADOR_H

#include <stdint.h> // Boa prática incluir para tipos de dados

// CORRIGIDO: Parâmetro 'msg' alterado para uint8_t, pois é mais apropriado.
// Parâmetros foram renomeados em ASK/FSK para maior clareza.
uint8_t modula_am(uint16_t freq_portadora, uint8_t msg);
uint8_t modula_fm(uint16_t freq_portadora, uint8_t msg);
uint8_t modula_ask(uint16_t taxa_bits, uint8_t msg);
uint8_t modula_fsk(uint16_t taxa_bits, uint8_t msg);
uint8_t gera_senoide_pura(uint16_t freq_hz);
void envia_dados(uint8_t y);

#endif // MODULADOR_H