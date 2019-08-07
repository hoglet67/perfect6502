#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>

#include "perfectz80.h"
#include "z80full.h"

static void *state = NULL;

static void sigint_handler(int signo) {
   printf("\nCtrl^C at cycle %d, exiting\n", cycle);
   shutdownChip(state);
   exit(0);
}

int main(int argc, char *argv[]) {

   if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fputs("An error occurred while setting a signal handler.\n", stderr);
      return EXIT_FAILURE;
   }

   state = initAndResetChip(argc, argv);

   // On reset
   memory[0] = 0x00; // NOP
   memory[1] = 0x00; // NOP
   memory[2] = 0x00; // NOP
   memory[3] = 0x31; // LD SP,0xFFFF
   memory[4] = 0xff;
   memory[5] = 0xff;
   memory[6] = 0xc3; // JMP 0x8000
   memory[7] = 0x00;
   memory[8] = 0x80;


   // RST 10 is character out
   memory[0x10] = 0xD3 ; // OUT (0xAA), A
   memory[0x11] = 0xAA ; // OUT (0xAA), A
   memory[0x12] = 0xC9 ; // OUT (0xAA), A

   // Fake CHAN-OPEN
   memory[0x1601] = 0xC9; // RET

   // Z80full loaded to 0x8000
   for (int i = 0; i < sizeof z80full_out; i++) {
      memory[0x8000 + i] = z80full_out[i];
   }

   /* Write the memory contents, for FPGA emulation */
   write_memory_to_file("test.bin");

   /* emulate the 6502! */
   for (;;) {
      step(state);
   }
}
