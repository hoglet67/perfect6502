OBJS_6502=perfect6502.o netlist_sim.o
OBJS_6502+=cbmbasic.o runtime.o runtime_init.o plugin.o console.o emu.o

OBJS_Z80=perfectz80.o netlist_sim.o

OBJS_Z80_BASIC=$(OBJS_Z80) z80basic.o
OBJS_Z80_FULL=$(OBJS_Z80) z80full.o
OBJS_Z80_DOC=$(OBJS_Z80) z80doc.o

#OBJS+=measure.o

CFLAGS=-Werror -Wall -O3
#CC=clang

all: z80basic z80full z80doc cbmbasic

cbmbasic: $(OBJS_6502)
	$(CC) -o cbmbasic $(OBJS_6502)

z80basic: $(OBJS_Z80_BASIC)
	$(CC) -o z80basic $(OBJS_Z80_BASIC)

z80full: $(OBJS_Z80_FULL)
	$(CC) -o z80full $(OBJS_Z80_FULL)

z80doc: $(OBJS_Z80_DOC)
	$(CC) -o z80doc $(OBJS_Z80_DOC)

clean:
	rm -f $(OBJS_Z80_FULL) $(OBJS_Z80_DOC) $(OBJS_Z80_BASIC) $(OBJS_6502)  z80full z80doc  z80basic cbmbasic

