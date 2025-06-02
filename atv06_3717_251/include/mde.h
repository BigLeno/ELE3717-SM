#ifndef MDE_H
#define MDE_H

#include <stdint.h>

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

void mde_init(void);
void mde_run(void);
state_t mde_get_state(void);
void mde_set_state(state_t new_state);
void mde_update_filter(void);

#endif // MDE_H
