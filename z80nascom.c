#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>
#include <readline/readline.h>

#include "perfectz80.h"

#include "z80nascom.h"

static void *state = NULL;

static void sigint_handler(int signo) {
   printf("\nCtrl^C at cycle %d, exiting\n", cycle);
   shutdownChip(state);
   exit(0);
}

char *input = "PRINT \"HELLO\"\r10 FOR A=0 TO 6.2 STEP 0.2\r20 PRINT TAB(40+SIN(A)*20);\"*\"\r30 NEXT A\rRUN\r";

int main(int argc, char *argv[]) {

   if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fputs("An error occurred while setting a signal handler.\n", stderr);
      return EXIT_FAILURE;
   }

   state = initAndResetChip(argc, argv);

   setUart(state, 0x80, 0x81);

   char *iptr = input;

   // z80nascom loaded to 0x0000
   for (int i = 0; i < sizeof z80nascom_bin; i++) {
      memory[i] = z80nascom_bin[i];
   }

   // Manually send the initial CR
   setUartRxChar(state, 13);
   setInt(state, 0);

   /* emulate the Z80 */
   for (;;) {
      step(state);
      memory[0x3000] = 0xff;
      if (getUartRxChar(state) < 0) {
         setInt(state, 1);
      }
      if (cycle > 700000) {
         if (*iptr && getUartRxChar(state) < 0 && (cycle % 10000) == 0) {
            setUartRxChar(state, *iptr++);
            setInt(state, 0);
         }
      }
   }
}
