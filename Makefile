OBJS_6502=perfect6502.o netlist_sim.o
OBJS_6502+=cbmbasic.o runtime.o runtime_init.o plugin.o console.o emu.o

OBJS_Z80=perfectz80.o netlist_sim.o tube.o

OBJS_Z80_BASIC=$(OBJS_Z80) z80basic.o
OBJS_Z80_FULL=$(OBJS_Z80) z80full.o
OBJS_Z80_DOC=$(OBJS_Z80) z80doc.o
OBJS_Z80_INTERRUPT=$(OBJS_Z80) z80interrupt.o
OBJS_Z80_TRAP1=$(OBJS_Z80) z80trap1.o
OBJS_Z80_NASCOM=$(OBJS_Z80) z80nascom.o
OBJS_Z80_CPM=$(OBJS_Z80) z80cpm.o
OBJS_Z80_HITCH=$(OBJS_Z80) z80hitch.o

#OBJS+=measure.o

#CFLAGS=-Werror -Wall -O3
#CC=clang

CFLAGS=-Wall -O3

all: z80basic z80full z80doc z80interrupt z80trap1 z80nascom z80cpm z80hitch cbmbasic

cbmbasic: $(OBJS_6502)
	$(CC) -o cbmbasic $(OBJS_6502)

z80basic: $(OBJS_Z80_BASIC)
	$(CC) -o z80basic $(OBJS_Z80_BASIC) -lreadline

z80full: $(OBJS_Z80_FULL)
	$(CC) -o z80full $(OBJS_Z80_FULL)

z80doc: $(OBJS_Z80_DOC)
	$(CC) -o z80doc $(OBJS_Z80_DOC)

z80interrupt: $(OBJS_Z80_INTERRUPT)
	$(CC) -o z80interrupt $(OBJS_Z80_INTERRUPT)

z80trap1: $(OBJS_Z80_TRAP1)
	$(CC) -o z80trap1 $(OBJS_Z80_TRAP1)

z80nascom: $(OBJS_Z80_NASCOM)
	$(CC) -o z80nascom $(OBJS_Z80_NASCOM)

z80cpm: $(OBJS_Z80_CPM)
	$(CC) -o z80cpm $(OBJS_Z80_CPM)

z80hitch: $(OBJS_Z80_HITCH)
	$(CC) -o z80hitch $(OBJS_Z80_HITCH)

clean:
	rm -f $(OBJS_Z80_FULL) $(OBJS_Z80_DOC) $(OBJS_Z80_BASIC) $(OBJS_Z80_INTERRUPT) $(OBJS_Z80_TRAP1) $(OBJS_Z80_NASCOM) $(OBJS_Z80_CPM) $(OBJS_Z80_HITCH) $(OBJS_6502)  z80full z80doc z80basic z80interrupt z80trap1 z80nascom z80cpm z80hitch cbmbasic

