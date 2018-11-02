#include <stdint.h>

#include "perfectz80.h"

extern void tube_reset(state_t *state, char *osword0_commands[], char *osrdch_commands[]);

extern void tube_clock(state_t *state);

extern void tube_write(int a, uint8_t d);

extern uint8_t tube_read(int a);


