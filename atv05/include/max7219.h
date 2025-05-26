#ifndef MAX7219_H
#define MAX7219_H

#include <stdint.h>

void max7219_init(void);
void max7219_send(uint8_t addr, uint8_t data);
void max7219_clear(void);

#endif // MAX7219_H