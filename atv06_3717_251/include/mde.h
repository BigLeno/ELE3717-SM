#ifndef MDE_H
#define MDE_H

#include <stdint.h>

// Estados da máquina de estados
typedef enum {
    STATE_INITIAL,
    STATE_BUTTONS
} state_t;

void mde_init(void);
void mde_run(void);
state_t mde_get_state(void);
void mde_set_state(state_t new_state);

#endif // MDE_H
