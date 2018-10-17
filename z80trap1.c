#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>
#include <readline/readline.h>

#include "perfectz80.h"

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

   int ptr = 0;

   setIntAckData(state, 0xF7); // RST 30

   // Reset
   memory[ptr++] = 0x31; // LD SP,0xEFFF
   memory[ptr++] = 0xff;
   memory[ptr++] = 0xef;
   memory[ptr++] = 0x3e; // LD A,0x00
   memory[ptr++] = 0x00;
   memory[ptr++] = 0xed; // LD I,A
   memory[ptr++] = 0x47;
   memory[ptr++] = 0xed; // IM 0
   memory[ptr++] = 0x46;
   memory[ptr++] = 0xfb; // EI
   memory[ptr++] = 0xc3; // JP 0x000A
   memory[ptr++] = 0x0A; //
   memory[ptr++] = 0x00; //

   // INT Handler (IM0) - max 8 bytes
   ptr = 0x0030;
   memory[ptr++] = 0xfb; // EI
   memory[ptr++] = 0xfb; // EI
   memory[ptr++] = 0xED; // RETI
   memory[ptr++] = 0x4D;

   // NMI Handler
   ptr = 0x0066;
   memory[ptr++] = 0xc3; // JP 0x0069 (so decoder detects an NMI)
   memory[ptr++] = 0x69; //
   memory[ptr++] = 0x00; //
   memory[ptr++] = 0xED; // LD A,I (PF set to IFF2)
   memory[ptr++] = 0x57; //
   memory[ptr++] = 0xF5; // PUSH AF
   memory[ptr++] = 0xF1; // POP AF
   memory[ptr++] = 0xED; // RETN
   memory[ptr++] = 0x45;

   /* emulate the Z80! */
   for (;;) {
      step(state);
   }
}
