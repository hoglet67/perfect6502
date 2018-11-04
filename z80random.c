#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>

#include "perfectz80.h"
#include "z80random.h"

static void *state = NULL;

static void sigint_handler(int signo) {
   printf("\nCtrl^C at cycle %d, exiting\n", cycle);
   shutdownChip(state);
   exit(0);
}

int main(int argc, char *argv[]) {

   int ncycles = 100000;

   if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fputs("An error occurred while setting a signal handler.\n", stderr);
      return EXIT_FAILURE;
   }

   state = initAndResetChip(argc, argv);

   setIntAckData(state, 0xF7); // RST 30

   srandom((unsigned int)get_user_param());

   // Client rom initially mapped to 0x0000
   for (int i = 0; i < sizeof z80random_bin; i++) {
      memory[0x0000 + i] = z80random_bin[i];
   }

   for (int i = 0x100; i < 0xffff; i++) {
      uint8_t opcode;
      do {
         opcode = (uint8_t) random();
      } while (opcode == 0x76);
      memory[i] = opcode;
   }

   /* Protect the code below 0x100 from accidental writes */
   setRamRange(state, 0x0100, 0xffff);

   /* emulate the Z80! */
   for (int i = 0; i < ncycles; i++) {
      step(state);
   }
   setNmi(state, 0);
   for (int i = 0; i < 1000; i++) {
      step(state);
   }

   shutdownChip(state);

}
