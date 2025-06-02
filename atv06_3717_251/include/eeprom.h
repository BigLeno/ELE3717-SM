#ifndef EEPROM_H
#define EEPROM_H

#include <stdint.h>

// Endereço base na EEPROM para os coeficientes
#define EEPROM_COEF_BASE_ADDR 0x00

// Funções básicas da EEPROM
void eeprom_write_data(uint16_t addr, uint8_t data);
uint8_t eeprom_read_data(uint16_t addr);

// Funções específicas para coeficientes
void eeprom_save_coefficients(uint8_t *coefficients, uint8_t count);
void eeprom_load_coefficients(uint8_t *coefficients, uint8_t count);
void eeprom_save_coefficient(uint8_t index, uint8_t value);
uint8_t eeprom_load_coefficient(uint8_t index);

#endif // EEPROM_H
