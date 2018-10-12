#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>

#include "perfectz80.h"

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
   setInt(state, 0);

   // On reset ...
   int mem_addr = 0;
   memory[mem_addr++] = 0x00; // NOP
   memory[mem_addr++] = 0x31; // LD SP,0x0080
   memory[mem_addr++] = 0x80;
   memory[mem_addr++] = 0x00;
   memory[mem_addr++] = 0x3e; // LD A,0xCC
   memory[mem_addr++] = 0xcc;
   memory[mem_addr++] = 0xed; // LD I,A
   memory[mem_addr++] = 0x47;
   memory[mem_addr++] = 0xed; // IM 2
   memory[mem_addr++] = 0x5e;
   memory[mem_addr++] = 0x00; // NOP
   memory[mem_addr++] = 0xfb; // EI

   // Incorrect vector points to 0x0100
   memory[0xCCC8] = 0x00;
   memory[0xCCC9] = 0x01;

   // Correcty vector point to 0x0180
   memory[0xCCE9] = 0x80;
   memory[0xCCEA] = 0x01;

   // Call then halt
   mem_addr = 0x0100;
   memory[mem_addr++] = 0xCD; // CALL 0x0103
   memory[mem_addr++] = 0x03;
   memory[mem_addr++] = 0x01;
   memory[mem_addr++] = 0x76; // HALT

   // Call then halt
   mem_addr = 0x0180;
   memory[mem_addr++] = 0xCD; // CALL 0x0183
   memory[mem_addr++] = 0x83;
   memory[mem_addr++] = 0x01;
   memory[mem_addr++] = 0x76; // HALT

   /* emulate the 6502! */
   chipStatus(state);
   for (int i = 0; i < 200; i++) {

      //printf("## ========================================================================\n");
      //printf("## PC=%04X\n",readPC(state));
      //chipStatus(state);
      //dump_node_state(state);
      //int tag = readPC(state);
      //dump_transistor_state(state, tag);

      step(state);

      cycle++;
   };
   chipStatus(state);
}
