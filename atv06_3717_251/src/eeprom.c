#include "eeprom.h"
#include <avr/io.h>
#include <avr/eeprom.h>

void eeprom_write_data(uint16_t addr, uint8_t data) {
    // Aguarda conclusão de escrita anterior
    while(EECR & (1 << EEPE));
    
    // Configura endereço
    EEAR = addr;
    // Configura dados
    EEDR = data;
    
    // Habilita escrita na EEPROM
    EECR |= (1 << EEMPE);
    EECR |= (1 << EEPE);
}

uint8_t eeprom_read_data(uint16_t addr) {
    // Aguarda conclusão de escrita anterior
    while(EECR & (1 << EEPE));
    
    // Configura endereço
    EEAR = addr;
    // Inicia leitura
    EECR |= (1 << EERE);
    
    // Retorna dados
    return EEDR;
}

void eeprom_save_coefficients(uint8_t *coefficients, uint8_t count) {
    for (uint8_t i = 0; i < count && i < 16; i++) {
        eeprom_write_data(EEPROM_COEF_BASE_ADDR + i, coefficients[i]);
    }
}

void eeprom_load_coefficients(uint8_t *coefficients, uint8_t count) {
    for (uint8_t i = 0; i < count && i < 16; i++) {
        coefficients[i] = eeprom_read_data(EEPROM_COEF_BASE_ADDR + i);
    }
}

void eeprom_save_coefficient(uint8_t index, uint8_t value) {
    if (index < 16) {
        eeprom_write_data(EEPROM_COEF_BASE_ADDR + index, value);
    }
}

uint8_t eeprom_load_coefficient(uint8_t index) {
    if (index < 16) {
        return eeprom_read_data(EEPROM_COEF_BASE_ADDR + index);
    }
    return 0;
}
