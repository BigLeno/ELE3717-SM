#ifndef MDE_H
#define MDE_H

#include <stdint.h>
#include "eeprom.h" // Adicionado include da EEPROM

// Estados da máquina de estados
typedef enum {
    STATE_INITIAL,
    STATE_BUTTONS,
    STATE_COEFFICIENTS
} state_t;

// Limites dos coeficientes
#define COEF_MIN 0
#define COEF_MAX 255
#define NUM_COEFFICIENTS 16

// Declarações extern para as flags de botões (definidas em main.c)
extern volatile uint8_t flag_btn_s1;
extern volatile uint8_t flag_btn_s2;
extern volatile uint8_t flag_btn_s3;

void mde_init(void);
void mde_run(void);
void mde_update_filter(void);
void mde_save_coefficients(void);
void mde_load_coefficients(void);
void mde_load_default_coefficients(void); // Nova função
state_t mde_get_state(void);
void mde_set_state(state_t new_state);

#endif // MDE_H
