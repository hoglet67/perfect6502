#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>
#include <readline/readline.h>

#include "perfectz80.h"

#include "tube.h"
#include "z80clientrom.h"
#include "z80hitchdisk.h"

static void *state = NULL;

// Commands to be fed to OSRDCH
// (must be terminated with \r)
static char *osrdch_commands[] = {
   "HITCH2\r",
   "TURN LIGHT ON\r",
   "GET UP\r",
   "GET GOWN\r",
   "WEAR GOWN\r",
   "LOOK IN POCKET\r",
   "GET ANALGESIC\r",
   "GET SCREWDRIVER AND TOOTHBRUSH\r",
   "S\r",
   "GET MAIL\r",
   "READ MAIL\r",
   "S\r",
   "LIE DOWN\r",
   "WAIT\r",
   "WAIT\r",
   "WAIT\r",
   "FORD, WHAT ABOUT MY HOME?\r",
   "WAIT\r",
   "WAIT\r",
   "S\r",
   "W\r",
   "DRINK BEER\r",
   "AGAIN\r",
   "AGAIN\r",
   "EXAMINE SHELF\r",
   "PAY FOR SANDWICH\r",
   "E\r",
   "FEED DOG\r",
   "WAIT\r",
   "WAIT\r",
   "WAIT\r",
   "WAIT\r",
   "WAIT\r",
   "WAIT\r",
   "WAIT\r",
   "WAIT\r",
   "EXAMINE SHIP\r",
   "GET DEVICE\r",
   "EXAMINE DEVICE\r",
   "PRESS GREEN\r",
   NULL
};

// Prompt to look for that preceeds feeding next osrdch_command
static char *osrdch_prompts[] = {
   "A>",
   "\n<",
   NULL
};

// Commands to be fed to OWORD0
// (must be terminated with \r)
static char *osword0_commands[] = {
   "CPM\r",
   NULL
};

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

   setTube(state, 1);

   setIntAckData(state, 0xFE); // IM 2 Vector

   // Client rom initially mapped to 0x0000
   for (int i = 0; i < sizeof z80clientrom_bin; i++) {
      memory[0x0000 + i] = z80clientrom_bin[i];
   }

   // Reset the tube emulation
   // Reset the tube emulation
   tube_reset(state, z80hitchdisk_bin, osword0_commands, osrdch_commands, osrdch_prompts);

   /* emulate the 6MHz Z80! */
   for (;;) {
      step(state);
      // Clock the tube at 2MHz
      if ((cycle % 6) == 0) {
         tube_clock(state);
      }
   }
}
