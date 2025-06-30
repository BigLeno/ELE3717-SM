
#ifndef MODULADOR_H
#define MODULADOR_H

uint16_t modula_am(uint16_t msg, uint8_t cont);
uint16_t modula_fm(uint16_t msg, uint8_t cont);
uint8_t modula_ask(uint16_t msg, uint8_t cont);
uint8_t modula_fsk(uint16_t msg, uint8_t cont);
void envia_dados(uint16_t y);

#endif
