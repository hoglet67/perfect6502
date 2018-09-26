#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <string.h>

#include "perfectz80.h"

#include "z80basic.h"

static void sigint_handler(int signo) {
   int len = 0x10;
   for (int i = 0; i < 0x10000; i++) {
      if ((i % len) == 0) {
         fprintf(stderr, "%04X :", i);
      }
      fprintf(stderr, " %02x", memory[i]);
      if ((i % len) == len - 1) {
         fprintf(stderr, "\n");
      }
   }
   fprintf(stderr, "\n");
   exit(0);
}

int main() {

   if (signal(SIGINT, sigint_handler) == SIG_ERR) {
      fputs("An error occurred while setting a signal handler.\n", stderr);
      return EXIT_FAILURE;
   }

   int cycle = 0;

   void *state = initAndResetChip();

   int ptr = 0;

   // Reset
   memory[ptr++] = 0x31; // LD SP,0xEFFF
   memory[ptr++] = 0xff;
   memory[ptr++] = 0xef;
   memory[ptr++] = 0xc3; // JP 0x100
   memory[ptr++] = 0x00;
   memory[ptr++] = 0x01;

   // PAGE = &3B00
   ptr = 0x3B00;
   memory[ptr++] = 0x07;
   memory[ptr++] = 0x0a;
   memory[ptr++] = 0x00;
   memory[ptr++] = 0xf1;
   memory[ptr++] = 0x20;
   memory[ptr++] = 0xaf;
   memory[ptr++] = 0x0d;
   memory[ptr++] = 0x05;
   memory[ptr++] = 0x14;
   memory[ptr++] = 0x00;
   memory[ptr++] = 0xe0;
   memory[ptr++] = 0x0d;
   memory[ptr++] = 0x00;
   memory[ptr++] = 0xFF;
   memory[ptr++] = 0xFF;

   // FFEE is OSWRCH
   ptr = 0xffee;
   memory[ptr++] = 0xD3 ; // OUT (0xAB), A
   memory[ptr++] = 0xAB ; //
   memory[ptr++] = 0xC9 ; // RET

   // FFF1 is OSWORD
   ptr = 0xfff1;
   memory[ptr++] = 0x37 ; // SCF
   memory[ptr++] = 0x3f ; // CCF
   memory[ptr++] = 0xc9 ; // RET

   // FFF4 is OSBYTE
   ptr = 0xfff4;
   memory[ptr++] = 0xc3; // JP 0xFF00
   memory[ptr++] = 0x00;
   memory[ptr++] = 0xff;

   // Simple OSBYTE Handler
   ptr = 0xff00;
   memory[ptr++] = 0xFE; // CP &83
   memory[ptr++] = 0x83;
   memory[ptr++] = 0x28; // JR Z +5
   memory[ptr++] = 0x05;
   memory[ptr++] = 0xFE; // CP &84
   memory[ptr++] = 0x84;
   memory[ptr++] = 0x28; // JR Z +5
   memory[ptr++] = 0x05;
   memory[ptr++] = 0xC9; // RET
   memory[ptr++] = 0x21; // LD HL, 0x3B00
   memory[ptr++] = 0x00;
   memory[ptr++] = 0x3B;
   memory[ptr++] = 0xC9; // RET
   memory[ptr++] = 0x21; // LD HL, 0xDC00
   memory[ptr++] = 0x00;
   memory[ptr++] = 0xDC;
   memory[ptr++] = 0xC9; // RET

   // Z80full loaded to 0x8000
   for (int i = 0; i < sizeof z80basic_bin; i++) {
      memory[0x0100 + i] = z80basic_bin[i];
   }

   char *commands[] = {
      "OLD\r",
      "PRINT \"Hello\"\r",
      "PRINT MID$(\"XX\"+\"XHE\"+\"LPXXX\",4,4)\r",
      "FOR I%=1 TO 3.7:PRINT I%:NEXT\r",
      "PRINT SIN(0.5)\r",
      "5 REM HELLO\r",
      "LIST\r",
      "REN. 1,2\r",
      "DIM P% 10:[NOP:RET\r",
      "LIST\r",
      "PRINT SQR(2)\r",
      "PRINT LOG(2)\r",
      "PRINT EXP(1)\r",
      "PRINT ACS(0)*2\r",
      "FOR I=1 TO 3:PRINT I:NEXT\r",
      "PRINT PI\r",
      "PRINT 22/7\r",
      "RUN\r",
   };

   int cmd = 0;

   /* emulate the 6502! */
   for (;;) {
      step(state);

      // OSWORD = &FFF1
      // INPUT BUFFER = &3800
      if (isFetchCycle(state, 0xfff1)) {
         if (cmd == sizeof(commands) / sizeof(char *)) {
            fprintf(stderr, "Run out of commands, exiting\n");
            exit(0);
         }
         fprintf(stderr, "%s\n", commands[cmd]);
         strcpy((char *)memory + 0x3800, commands[cmd++]);
         memory[0x3B00] = 0x07;
      }

      //if (cycle % 1000000 == 1) {
      //  chipStatus(state);
      //}

      cycle++;
   };
}
