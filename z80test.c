#include <stdio.h>
#include <inttypes.h>

#include "perfectz80.h"

#include "z80full.h"

int main() {

   int clock = 0;
   int cycle = 0;

   void *state = initAndResetChip();

   // On reset JP 0x8000
   memory[0] = 0xcd;
   memory[1] = 0x00;
   memory[2] = 0x80;

   // RST 10 is character out
   memory[0x10] = 0xD3 ; // OUT (0xAA), A
   memory[0x11] = 0xAA ; // OUT (0xAA), A
   memory[0x12] = 0xC9 ; // OUT (0xAA), A

   // Fake CHAN-OPEN
   memory[0x1601] = 0xC9; // RET

   // Z80full loaded to 0x8000
   for (int i = 0; i < sizeof z80full_bin; i++) {
      memory[0x8000 + i] = z80full_bin[i];
   }

   /* emulate the 6502! */
   for (;;) {
      step(state);

      //if (cycle % 1000000 == 1) {
      //  chipStatus(state);
      //}

      clock = !clock;
      cycle++;

      //if (!(cycle % 1000000)) printf("%d\n", cycle);
   };
}
