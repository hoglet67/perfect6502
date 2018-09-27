#ifndef INCLUDED_FROM_NETLIST_SIM_C
#define state_t void
#endif

extern state_t *initAndResetChip(int argc, char *argv[]);
extern void shutdownChip(state_t *state);
extern void step(state_t *state);
extern void chipStatus(state_t *state);
extern unsigned short readPC(state_t *state);
extern unsigned char readA(state_t *state);
extern unsigned char readF(state_t *state);
extern unsigned char readB(state_t *state);
extern unsigned char readC(state_t *state);
extern unsigned char readD(state_t *state);
extern unsigned char readE(state_t *state);
extern unsigned char readH(state_t *state);
extern unsigned char readL(state_t *state);
extern unsigned short readIX(state_t *state);
extern unsigned short readIY(state_t *state);
extern unsigned short readSP(state_t *state);

extern unsigned int readM1(state_t *state);
extern unsigned int readRD(state_t *state);
extern unsigned int readWR(state_t *state);
extern unsigned int readMREQ(state_t *state);
extern unsigned int readIORQ(state_t *state);
extern unsigned short readAddressBus(state_t *state);
extern void writeDataBus(state_t *state, unsigned char);
extern unsigned char readDataBus(state_t *state);

extern unsigned char memory[65536];
extern unsigned int cycle;
extern unsigned int transistors;

extern int isFetchCycle(void *state, unsigned int addr);

extern void dump_memory();
