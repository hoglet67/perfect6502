#ifndef INCLUDED_FROM_NETLIST_SIM_C
#define state_t void
#endif

extern state_t *initAndResetChip(int argc, char *argv[]);
extern void setIntAckData(state_t *state, int value);
extern void setInt(state_t *state, int value);
extern void setNmi(state_t *state, int value);
extern void setUart(state_t *data, int control_addr, int data_addr);
extern void setUartRxChar(state_t *state, int c);
extern int  getUartRxChar(state_t *state);
extern void setTube(state_t *state, int value);
extern void setRamRange(state_t *state, uint16_t lo, uint16_t hi);
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
extern unsigned char readW(state_t *state);
extern unsigned char readZ(state_t *state);
extern unsigned short readWZ(state_t *state);
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
extern int cycle;
extern unsigned int transistors;

extern int isFetchCycle(void *state, unsigned int addr);

extern void dump_memory();
extern void dump_registers(state_t *state);
extern void dump_node_state(state_t *state, int tag);
extern void dump_transistor_state(state_t *state, int tag);

extern int get_user_param();

extern void write_memory_to_file(char *filename);
