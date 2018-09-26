OBJS_6502=perfect6502.o netlist_sim.o
OBJS_6502+=cbmbasic.o runtime.o runtime_init.o plugin.o console.o emu.o

OBJS_Z80=perfectz80.o netlist_sim.o
OBJS_Z80_TEST=$(OBJS_Z80) z80test.o
OBJS_Z80_BASIC=$(OBJS_Z80) z80basic.o

#OBJS+=measure.o

CFLAGS=-Werror -Wall -O3
#CC=clang

all: z80basic z80test cbmbasic

cbmbasic: $(OBJS_6502)
	$(CC) -o cbmbasic $(OBJS_6502)

z80test: $(OBJS_Z80_TEST)
	$(CC) -o z80test $(OBJS_Z80_TEST)

z80basic: $(OBJS_Z80_BASIC)
	$(CC) -o z80basic $(OBJS_Z80_BASIC)

clean:
	rm -f $(OBJS_6502) $(OBJS_Z80_TEST) $(OBJS_Z80_BASIC) cbmbasic z80test z80basic

