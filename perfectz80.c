/*
 Copyright (c) 2010,2014 Michael Steil, Brian Silverman, Barry Silverman

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 */

#include <stdio.h>
#include "types.h"
#include "netlist_sim.h"
/* nodes & transistors */
#include "netlist_z80.h"

/************************************************************
 *
 * Z80-specific Interfacing
 *
 ************************************************************/

uint16_t
readAddressBus(void *state)
{
   return readNodes(state, 16, (nodenum_t[]){ ab0, ab1, ab2, ab3, ab4, ab5, ab6, ab7, ab8, ab9, ab10, ab11, ab12, ab13, ab14, ab15 });
}

uint8_t
readDataBus(void *state)
{
   return readNodes(state, 8, (nodenum_t[]){ db0, db1, db2, db3, db4, db5, db6, db7 });
}

void
writeDataBus(void *state, uint8_t d)
{
   writeNodes(state, 8, (nodenum_t[]){ db0, db1, db2, db3, db4, db5, db6, db7 }, d);
}

BOOL
readM1(void *state)
{
   return isNodeHigh(state, _m1);
}

BOOL
readRD(void *state)
{
   return isNodeHigh(state, _rd);
}

BOOL
readWR(void *state)
{
   return isNodeHigh(state, _wr);
}

BOOL
readMREQ(void *state)
{
   return isNodeHigh(state, _mreq);
}

BOOL
readIORQ(void *state)
{
   return isNodeHigh(state, _iorq);
}

uint8_t
readA(void *state)
{
   if (!isNodeHigh(state, ex_af)) {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_aa0,reg_aa1,reg_aa2,reg_aa3,reg_aa4,reg_aa5,reg_aa6,reg_aa7 });
   } else {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_a0,reg_a1,reg_a2,reg_a3,reg_a4,reg_a5,reg_a6,reg_a7 });
   }
}

uint8_t
readF(void *state)
{
   if (!isNodeHigh(state, ex_af)) {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_ff0,reg_ff1,reg_ff2,reg_ff3,reg_ff4,reg_ff5,reg_ff6,reg_ff7 });
   } else {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_f0,reg_f1,reg_f2,reg_f3,reg_f4,reg_f5,reg_f6,reg_f7 });
   }
}

uint8_t
readB(void *state)
{
   if (isNodeHigh(state, ex_bcdehl)) {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_bb0,reg_bb1,reg_bb2,reg_bb3,reg_bb4,reg_bb5,reg_bb6,reg_bb7 });
   } else {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_b0,reg_b1,reg_b2,reg_b3,reg_b4,reg_b5,reg_b6,reg_b7 });
   }
}

uint8_t
readC(void *state)
{
   if (isNodeHigh(state, ex_bcdehl)) {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_cc0,reg_cc1,reg_cc2,reg_cc3,reg_cc4,reg_cc5,reg_cc6,reg_cc7 });
   } else {
      return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_c0,reg_c1,reg_c2,reg_c3,reg_c4,reg_c5,reg_c6,reg_c7 });
   }
}

uint8_t
readD(void *state)
{
   if (isNodeHigh(state, ex_bcdehl)) {
      if (isNodeHigh(state, ex_dehl1)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_hh0,reg_hh1,reg_hh2,reg_hh3,reg_hh4,reg_hh5,reg_hh6,reg_hh7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_dd0,reg_dd1,reg_dd2,reg_dd3,reg_dd4,reg_dd5,reg_dd6,reg_dd7 });
      }
   } else {
      if (isNodeHigh(state, ex_dehl0)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_h0,reg_h1,reg_h2,reg_h3,reg_h4,reg_h5,reg_h6,reg_h7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_d0,reg_d1,reg_d2,reg_d3,reg_d4,reg_d5,reg_d6,reg_d7 });
      }
   }
}

uint8_t
readE(void *state)
{
   if (isNodeHigh(state, ex_bcdehl)) {
      if (isNodeHigh(state, ex_dehl1)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_ll0,reg_ll1,reg_ll2,reg_ll3,reg_ll4,reg_ll5,reg_ll6,reg_ll7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_ee0,reg_ee1,reg_ee2,reg_ee3,reg_ee4,reg_ee5,reg_ee6,reg_ee7 });
      }
   } else {
      if (isNodeHigh(state, ex_dehl0)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_l0,reg_l1,reg_l2,reg_l3,reg_l4,reg_l5,reg_l6,reg_l7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_e0,reg_e1,reg_e2,reg_e3,reg_e4,reg_e5,reg_e6,reg_e7 });
      }
   }
}

uint8_t
readH(void *state)
{
   if (isNodeHigh(state, ex_bcdehl)) {
      if (isNodeHigh(state, ex_dehl1)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_dd0,reg_dd1,reg_dd2,reg_dd3,reg_dd4,reg_dd5,reg_dd6,reg_dd7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_hh0,reg_hh1,reg_hh2,reg_hh3,reg_hh4,reg_hh5,reg_hh6,reg_hh7 });
      }
   } else {
      if (isNodeHigh(state, ex_dehl0)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_d0,reg_d1,reg_d2,reg_d3,reg_d4,reg_d5,reg_d6,reg_d7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_h0,reg_h1,reg_h2,reg_h3,reg_h4,reg_h5,reg_h6,reg_h7 });
      }
   }
}

uint8_t
readL(void *state)
{
   if (isNodeHigh(state, ex_bcdehl)) {
      if (isNodeHigh(state, ex_dehl1)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_ee0,reg_ee1,reg_ee2,reg_ee3,reg_ee4,reg_ee5,reg_ee6,reg_ee7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_ll0,reg_ll1,reg_ll2,reg_ll3,reg_ll4,reg_ll5,reg_ll6,reg_ll7 });
      }
   } else {
      if (isNodeHigh(state, ex_dehl0)) {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_e0,reg_e1,reg_e2,reg_e3,reg_e4,reg_e5,reg_e6,reg_e7 });
      } else {
         return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_l0,reg_l1,reg_l2,reg_l3,reg_l4,reg_l5,reg_l6,reg_l7 });
      }
   }
}

uint8_t
readIXL(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_ixl0,reg_ixl1,reg_ixl2,reg_ixl3,reg_ixl4,reg_ixl5,reg_ixl6,reg_ixl7 });
}

uint8_t
readIXH(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_ixh0,reg_ixh1,reg_ixh2,reg_ixh3,reg_ixh4,reg_ixh5,reg_ixh6,reg_ixh7 });
}


uint8_t
readIYL(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_iyl0,reg_iyl1,reg_iyl2,reg_iyl3,reg_iyl4,reg_iyl5,reg_iyl6,reg_iyl7 });
}

uint8_t
readIYH(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_iyh0,reg_iyh1,reg_iyh2,reg_iyh3,reg_iyh4,reg_iyh5,reg_iyh6,reg_iyh7 });
}

uint8_t
readSPL(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_spl0,reg_spl1,reg_spl2,reg_spl3,reg_spl4,reg_spl5,reg_spl6,reg_spl7 });
}

uint8_t
readSPH(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_sph0,reg_sph1,reg_sph2,reg_sph3,reg_sph4,reg_sph5,reg_sph6,reg_sph7 });
}

uint8_t
readPCL(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_pcl0,reg_pcl1,reg_pcl2,reg_pcl3,reg_pcl4,reg_pcl5,reg_pcl6,reg_pcl7 });
}

uint8_t
readPCH(void *state)
{
   return 0xFF ^ readNodes(state, 8, (nodenum_t[]){ reg_pch0,reg_pch1,reg_pch2,reg_pch3,reg_pch4,reg_pch5,reg_pch6,reg_pch7 });
}


uint16_t
readIX(void *state)
{
   return (readIXH(state) << 8) | readIXL(state);
}

uint16_t
readIY(void *state)
{
   return (readIYH(state) << 8) | readIYL(state);
}

uint16_t
readSP(void *state)
{
   return (readSPH(state) << 8) | readSPL(state);
}

uint16_t
readPC(void *state)
{
   return (readPCH(state) << 8) | readPCL(state);
}

/************************************************************
 *
 * Address Bus and Data Bus Interface
 *
 ************************************************************/

uint8_t memory[65536];

static uint8_t
mRead(uint16_t a)
{
   return memory[a];
}

static void
mWrite(uint16_t a, uint8_t d)
{
   memory[a] = d;
}

static inline void
handleMemory(void *state)
{
   int m1 = isNodeHigh(state, _m1);
   int mreq = isNodeHigh(state, _mreq);
   int iorq = isNodeHigh(state, _iorq);
   int rd = isNodeHigh(state, _rd);
   int wr = isNodeHigh(state, _wr);
   static int last_wr = 1;

   // Memory Read
   if (!mreq && !rd) {
      writeDataBus(state, mRead(readAddressBus(state)));
   }

   // Memory Write
   if (!mreq && !wr) {
      mWrite(readAddressBus(state), readDataBus(state));
   }

   // IO Read
   if (!iorq && !rd) {
      writeDataBus(state, 0x00);
   }

   // IO Write (falling edge only)
   if (!iorq && !wr && last_wr) {
      int a = readAddressBus(state) & 0xff;
      int d = readDataBus(state);
      if (a == 0xAA) {
         // Spectrum Output
         if (d < 32) {
            if (d == 13) {
               fprintf(stderr, "\n");
            } else {
               fprintf(stderr, ".");
            }
         } else if (d == 127) {
            fprintf(stderr, "©");
         } else {
            fprintf(stderr, "%c", d);
         }
         fflush(stderr);
      } else if (a == 0xab) {
         // Acorn output
         fprintf(stderr, "%c", d);
         fflush(stderr);
      } else if (a == 0xfe) {
         // Mic??
      } else {
         fprintf(stderr, "IO Write: %02x=%02x\n", a, d);
      }
   }

   // Output same for analysis with Z80Decoder
   putchar(readDataBus(state));
   putchar(m1 | (rd << 1) | (wr << 2) | (mreq << 3) | (iorq << 4) | 0xE0);

   last_wr = wr;
}

/************************************************************
 *
 * Main Clock Loop
 *
 ************************************************************/

static unsigned int cycle;

void
step(void *state)
{
   BOOL clock = isNodeHigh(state, clk);

   /* invert clock */
   setNode(state, clk, !clock);
   recalcNodeList(state);

   /* handle memory reads and writes */
   if (!clock)
      handleMemory(state);

   cycle++;
}

void *
initAndResetChip()
{
   /* set up data structures for efficient emulation */
   nodenum_t nodes = sizeof(netlist_z80_node_is_pullup)/sizeof(*netlist_z80_node_is_pullup);
   nodenum_t transistors = sizeof(netlist_z80_transdefs)/sizeof(*netlist_z80_transdefs);
   void *state = setupNodesAndTransistors(netlist_z80_transdefs,
                                 netlist_z80_node_is_pullup,
                                 nodes,
                                 transistors,
                                 vss,
                                 vcc);

   setNode(state, _reset, 0);
   setNode(state, clk,    1);
   setNode(state, _busrq, 1);
   setNode(state, _int,   1);
   setNode(state, _nmi,   1);
   setNode(state, _wait,  1);

   stabilizeChip(state);

   /* hold RESET for 32 cycles */
   for (int i = 0; i < 32; i++)
      step(state);

   /* release RESET */
   setNode(state, _reset, 1);
   recalcNodeList(state);

   cycle = 0;

   return state;
}

int isFetchCycle(void *state, unsigned int addr) {
   static BOOL prev_condition = 0;
   uint16_t a = readAddressBus(state);
   BOOL m1    = isNodeHigh(state, _m1);
   BOOL rd    = isNodeHigh(state, _rd);
   BOOL mreq  = isNodeHigh(state, _mreq);
   BOOL condition = !m1 && !mreq && !rd && (a == addr);
   BOOL result = condition & !prev_condition;
   prev_condition = condition;
   return result;
}

/************************************************************
 *
 * Tracing/Debugging
 *
 ************************************************************/

void
chipStatus(void *state)
{
   BOOL clock = isNodeHigh(state, clk);
   uint16_t a = readAddressBus(state);
   uint8_t d  = readDataBus(state);
   BOOL m1    = isNodeHigh(state, _m1);
   BOOL rd    = isNodeHigh(state, _rd);
   BOOL wr    = isNodeHigh(state, _wr);
   BOOL mreq  = isNodeHigh(state, _mreq);
   BOOL iorq  = isNodeHigh(state, _iorq);

   printf("halfcyc:%d clk:%d AB:%04X D:%02X CTRL:%d%d%d%d%d PC:%04X AF:%02X%02X BC:%02X%02X DE:%02X%02X HL:%02X%02X IX:%04X IY:%04X SP:%04X",
         cycle,
         clock,
         a,
         d,
         m1,
         rd,
         wr,
         mreq,
         iorq,
         readPC(state),
         readA(state),
         readF(state),
         readB(state),
         readC(state),
         readD(state),
         readE(state),
         readH(state),
         readL(state),
         readIX(state),
         readIY(state),
         readSP(state));

   if (clock && !mreq && !rd) {
      printf(" R$%04X=$%02X", a, memory[a]);
   }
   if (clock && !mreq && !wr) {
      printf(" W$%04X=$%02X", a, d);
   }
   if (clock && !iorq && !rd) {
      printf(" IOR$%04X=$%02X", a, memory[a]);
   }
   if (clock && !iorq && !wr) {
      printf(" IOW$%04X=$%02X", a, d);
   }

   printf("\n");
}