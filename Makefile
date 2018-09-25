OBJS_6502=perfect6502.o netlist_sim.o
OBJS_6502+=cbmbasic.o runtime.o runtime_init.o plugin.o console.o emu.o

OBJS_Z80=perfectz80.o netlist_sim.o
OBJS_Z80+=z80test.o

#OBJS+=measure.o

CFLAGS=-Werror -Wall -O3
#CC=clang

all: cbmbasic z80test

cbmbasic: $(OBJS_6502)
	$(CC) -o cbmbasic $(OBJS_6502)

z80test: $(OBJS_Z80)
	$(CC) -o z80test $(OBJS_Z80)

clean:
	rm -f $(OBJS_6502) $(OBJS_Z80) cbmbasic z80test

