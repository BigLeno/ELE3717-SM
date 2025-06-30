#ifndef BTN_H
#define BTN_H

#include <stdint.h>

// Botões nos pinos PC1, PC2, PC3
#define BTN_S1 PC1
#define BTN_S2 PC2
#define BTN_S3 PC3

void btn_init(void);
uint8_t btn_read(uint8_t btn);

#endif // BTN_H
