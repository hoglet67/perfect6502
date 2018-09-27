#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>

#include "perfectz80.h"
#include "z80doc.h"

static void *state = NULL;

static void sigint_handler(int signo) {
   dump_memory();
   shutdownChip(state);
   exit(0);
}

int main(int argc, char *argv[]) {

   int cycle = 0;

   if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fputs("An error occurred while setting a signal handler.\n", stderr);
      return EXIT_FAILURE;
   }

   state = initAndResetChip(argc, argv);

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

   // Z80doc loaded to 0x8000
   for (int i = 0; i < sizeof z80doc_bin; i++) {
      memory[0x8000 + i] = z80doc_bin[i];
   }

   /* emulate the 6502! */
   for (;;) {
      step(state);

      //if (cycle % 1000000 == 1) {
      //  chipStatus(state);
      //}

      cycle++;
   };
}
