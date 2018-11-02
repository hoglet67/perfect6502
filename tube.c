#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

#include "tube.h"
#include "z80cpmdisk.h"

static uint8_t host_memory[0x10000];

static int tube_cycles;

#define TR_IDLE     0
#define TR_REQUEST  1
#define TR_TRANSFER 2
#define TR_RELEASE  3

// #define DEBUG

enum R2_enum {
   R2_IDLE,
   R2_OSCLI_0,
   R2_OSBYTELO_0,
   R2_OSBYTELO_1,
   R2_OSBYTEHI_0,
   R2_OSBYTEHI_1,
   R2_OSBYTEHI_2,
   R2_OSWORD_0,
   R2_OSWORD_1,
   R2_OSWORD_2,
   R2_OSWORD_3,
   R2_OSWORD0_0,
   R2_OSWORD0_1,
   R2_OSWORD0_2,
   R2_OSWORD0_3,
   R2_OSWORD0_4,
   R2_OSARGS_0,
   R2_OSARGS_1,
   R2_OSARGS_2,
   R2_OSARGS_3,
   R2_OSBGET_0,
   R2_OSBPUT_0,
   R2_OSBPUT_1,
   R2_OSFIND_0,
   R2_OSFIND_1,
   R2_OSFIND_2,
   R2_OSFILE_0,
   R2_OSFILE_1,
   R2_OSFILE_2,
   R2_OSGBPB_0,
   R2_OSGBPB_1
};

state_t *sim_state;

int      transfer_state = TR_IDLE;
uint8_t *transfer_ptr = NULL;
int      transfer_len = 0;

int      delayed_response_len = 0;
uint8_t  delayed_response_data[256];

static char *osrdch_input_ptr = NULL;
static int osrdch_input_line = 0;
static char **osrdch_input;

static int osword0_input_line = 0;
static char **osword0_input;

// Making the FIFOs large simplifies the host
#define FIFO_SIZE 256

typedef struct {
   int id;
   uint8_t buffer[FIFO_SIZE];
   int wr_index;
   int rd_index;
} fifo_type;

fifo_type r1;
fifo_type r2;
fifo_type r3;
fifo_type r4;

void fifo_init(fifo_type *fifo, int id) {
   fifo->id = id;
   fifo->wr_index = 0;
   fifo->rd_index = 0;
}

int fifo_empty(fifo_type *fifo) {
   return fifo->wr_index == fifo->rd_index;
}

int fifo_full(fifo_type *fifo) {
   return (fifo->wr_index + 1) % FIFO_SIZE == fifo->rd_index;
}

void fifo_write(fifo_type *fifo, uint8_t data) {
   if (fifo_full(fifo)) {
      printf("fifo_write: fifo r%d is full\n", fifo->id);
   } else {
      fifo->buffer[fifo->wr_index] = data;
      fifo->wr_index = (fifo->wr_index + 1) % FIFO_SIZE;
   }
}

uint8_t fifo_read(fifo_type *fifo) {
   uint8_t data = 0xEE;
   if (fifo_empty(fifo)) {
      printf("read_fifo: fifo r%d is empty\n", fifo->id);
   } else {
      data = fifo->buffer[fifo->rd_index];
      fifo->rd_index = (fifo->rd_index + 1) % FIFO_SIZE;
   }
   return data;
}

void tube_reset(state_t *state, char *osword0_commands[], char *osrdch_commands[]) {
   // Save the arguments
   sim_state = state;
   osword0_input = osword0_commands;
   osword0_input_line = 0;
   osrdch_input = osrdch_commands;
   osrdch_input_line = 0;
   // So we see output quickly if piping into tee
   setlinebuf(stdout);
   tube_cycles = 0;
   // Clear the response buffers
   fifo_init(&r1, 1);
   fifo_init(&r2, 2);
   fifo_init(&r3, 3);
   fifo_init(&r4, 4);
   // Preload the response to the reset mesage
   fifo_write(&r2, 0x7f);
}

void tube_clock(state_t *state) {
   tube_cycles++;
   if (fifo_empty(&r1) && fifo_empty(&r4)) {
      setInt(state, 1);
   } else {
      setInt(state, 0);
   }

   switch(transfer_state) {

   case TR_REQUEST:
      // Wait for R4 to become empty (i.e. sync byte read)
      if (fifo_empty(&r4)) {
         transfer_state = TR_TRANSFER;
         tube_cycles = 0;
      }
      break;

   case TR_TRANSFER:
      // tube_cycles increments at 2MHz
      // the maximum rate of NMI's is one every 24us, so lets be cautious...and generate one every 32 us.

      if (transfer_len > 0) {
         if ((tube_cycles % 48) == 24) {
            fifo_write(&r3, *transfer_ptr++);
            transfer_len--;
            setNmi(state, 0);
         } else if ((tube_cycles % 48) == 47) {
            setNmi(state, 1);
         }
      } else {
         setNmi(state, 1);
         transfer_state = TR_RELEASE;
      }
      break;

   case TR_RELEASE:
      for (int i = 0; i < delayed_response_len; i++) {
         fifo_write(&r2, delayed_response_data[i]);
      }
      transfer_state = TR_IDLE;
      break;
   }
}

void initiate_transfer(int type, int id, uint32_t address, uint8_t *data, int length) {
   // Queue the data transfer request to R4
   fifo_write(&r4, type); // action
   fifo_write(&r4, id); // id
   fifo_write(&r4, (address >> 24) & 0xFF); // addr
   fifo_write(&r4, (address >> 16) & 0xFF); // addr
   fifo_write(&r4, (address >>  8) & 0xFF); // addr
   fifo_write(&r4, (address >>  0) & 0xFF); // addr
   fifo_write(&r4, 0x55); // sync

   // the rest happens asyncronously
   transfer_state = TR_REQUEST;
   transfer_ptr   = data;
   transfer_len   = length;
}

void do_osword(uint8_t a, uint8_t *block, int in_length, int out_length) {
   // Response <block>
   in_length--;

   if (a == 0x05) {
      // read byte from host memory
      uint32_t address = block[in_length] | (block[in_length - 1] << 8) | (block[in_length - 2] << 16) | (block[in_length - 3] << 24);
      block[0] = host_memory[address & 0xffff];

   } else if (a == 0x06) {
      // write to byte of host memory
      uint32_t address = block[in_length] | (block[in_length - 1] << 8) | (block[in_length - 2] << 16) | (block[in_length - 3] << 24);
      host_memory[address & 0xffff] = block[0];

   } else if (a == 0x7F) {
      uint8_t drive = block[in_length];
      uint32_t address = block[in_length - 1] | (block[in_length - 2] << 8) | (block[in_length - 3] << 16) | (block[in_length - 4] << 24);
      uint8_t nparams = block[in_length - 5];
      uint8_t command = block[in_length - 6];
      int track;
      int sector;
      int size;
      int count;
      int offset;
      int length;
      switch (command) {
      case 0x53:
      case 0x57:
         // Read sectors
         track = block[in_length - 7];
         sector = block[in_length - 8];
         size = 128 << ((block[in_length - 9] & 0xE0) >> 5);
         count = block[in_length - 9] & 0x1F;
         printf("Reading drive %d track %d sector %d size %d count %d\n",
                drive, track, sector, size, count);
         // Assume 10 sectors per track
         offset = size * (((drive & 2) ? 10 : 0) + (track * 20) + sector);
         length = size * count;
         // Generate the response
         if (offset + length < z80cpmdisk_bin_len) {

            // Pre-generate a success response
            block[in_length - 7 - nparams] = 0x00;

            if ((address >> 16) == 0xFFFF) {
               address &= 0xffff;
               if ((length + address) >= 0x10000) {
                  length = 0x10000 - address;
               }
               memcpy(host_memory + (address & 0xffff), z80cpmdisk_bin + offset, length);

            } else {

               // setup the delayed response to the OSWORD
               delayed_response_len = out_length;
               memcpy(delayed_response_data, block, out_length);

               initiate_transfer(1, 0xcc, address, z80cpmdisk_bin + offset, length);

               return;
            }

         } else {
            printf("Oops, attempt to read beyond end of disk\n");
            // Generate a success response
            block[in_length - 7 - nparams] = 0xFF;
         }
         break;
      default:
         printf("Unsupported OSWORD A=7F command: %02x\n", command);
      }
   } else if (a == 0xFF) {

      // R2: OSWORD: A=ff BLOCK=01 02 00 00 00 f2 72 00 00 26 00 01 0d
      int command = block[in_length - 12];
      int length = block[in_length - 10] + (block[in_length - 11] << 8);
      uint32_t host_address = block[in_length - 2] | (block[in_length - 3] << 8) | (block[in_length - 4] << 16) | (block[in_length - 5] << 24);
      uint32_t para_address = block[in_length - 6] | (block[in_length - 7] << 8) | (block[in_length - 8] << 16) | (block[in_length - 9] << 24);

      // setup the delayed response to the OSWORD
      delayed_response_len = 1;
      memcpy(delayed_response_data, block + in_length, delayed_response_len);

      switch (command) {
      case 0:
         // Transfer host to parasite
         initiate_transfer(0, 0xcc, para_address, host_memory + host_address, length);
         return;
      case 1:
         // Transfer host to parasite
         initiate_transfer(1, 0xcc, para_address, host_memory + host_address, length);
         return;
      default:
         printf("Unsupported OSWORD A=FF command: %02x\n", command);
      }

   } else {
      printf("OSWORD A=%02x not implemented\n", a);
   }
   for (int i = 0; i < out_length; i++) {
      fifo_write(&r2, block[i]);
   }
}

void do_osword0(uint8_t *block) {
   char *command = osword0_input[osword0_input_line++];
   if (command) {
      printf("OSWORD0: %s\n", command);
      // Response FF|7F <string> <0D>
      fifo_write(&r2, 0x7F);
      for (int i = 0; i < strlen(command); i++) {
         fifo_write(&r2, command[i]);
      }
   } else {
      printf("OSWORD0 ran out of input, exiting on cycle %d\n", cycle);
      shutdownChip(sim_state);
      exit(0);
   }
}

void do_osrdch() {
   int c = *osrdch_input_ptr;
   if (c) {
      // Response Cy A
      fifo_write(&r2, 0x00);
      fifo_write(&r2, c);
      osrdch_input_ptr++;
   } else {
      printf("Ooops, seem to have read beyond the available input\n");
   }
}

void do_oswrch(uint8_t a) {
   static int last_a = -1;
   printf("R1: OSWRCH: %c <%02x>\n", (a >= 32 && a < 127) ? a : '.', a);
   // Nasty hack: at the A> propmt, make the next line of input available
   if (last_a == 'A' && a == '>') {
      osrdch_input_ptr = osrdch_input[osrdch_input_line++];
      if (osrdch_input_ptr == NULL) {
         printf("OSRDCH ran out of input, exiting\n");
         shutdownChip(sim_state);
         exit(0);
      }
   }
   last_a = a;
}

void do_oscli(uint8_t *command) {
   printf("OSCLI not implemented\n");
   // Response &7F or &80
   fifo_write(&r2, 0x7F);
}

void do_osbytelo(uint8_t a, uint8_t x) {
   printf("OSBYTE not implemented\n");
   // Response X
   fifo_write(&r2, 0x00);
}

void do_osbytehi(uint8_t a, uint8_t x, uint8_t y) {
   // Response CY Y X
   if (a == 0xb1) {
      // Read input source
      fifo_write(&r2, 0x00);
      fifo_write(&r2, 0x00);
      fifo_write(&r2, 0x00);
   } else if (a == 0x98) {
      int empty = !osrdch_input_ptr || !(*osrdch_input_ptr);
      fifo_write(&r2, empty ? 0x80 : 0x00);
      fifo_write(&r2, 0x00);
      fifo_write(&r2, 0x00);
   } else {
      printf("OSBYTE A=%02x not implemented\n", a);
      fifo_write(&r2, 0x00);
      fifo_write(&r2, 0x00);
      fifo_write(&r2, 0x00);
   }
}

void do_osargs(uint8_t a, uint8_t y, uint8_t *block) {
   printf("OSARGS not implemented\n");
   // Response A <block>
   fifo_write(&r2, 0x00);
   fifo_write(&r2, 0x00);
   fifo_write(&r2, 0x00);
   fifo_write(&r2, 0x00);
   fifo_write(&r2, 0x00);
}

void do_osbget(uint8_t y) {
   printf("OSBGET not implemented\n");
   // Response Cy A
   fifo_write(&r2, 0x00);
   fifo_write(&r2, 0x00);
}

void do_osbput(uint8_t a, uint8_t y) {
   printf("OSBPUT not implemented\n");
   // Response 0x7F
   fifo_write(&r2, 0x7F);
}

void do_osfind0(uint8_t y) {
   printf("OSFIND not implemented\n");
   // Response 0x7F
   fifo_write(&r2, 0x7F);
}

void do_osfindA(uint8_t a, uint8_t *string) {
   printf("OSFIND not implemented\n");
   // Response A
   fifo_write(&r2, 0x00);
}

void do_osfile(uint8_t a, uint8_t *block, uint8_t *string) {
   printf("OSFILE not implemented\n");
   // Response A <block>
   fifo_write(&r2, 0x00);
   for (int i = 0; i < 16; i++) {
      fifo_write(&r2, 0x00);
   }
}

void do_osgbpb(uint8_t a, uint8_t *block) {
   printf("OSGBPB not implemented\n");
   // Response <block> Cy A
   for (int i = 0; i < 16; i++) {
      fifo_write(&r2, 0x00);
   }
   fifo_write(&r2, 0x00);
   fifo_write(&r2, a);
}

static void print_call(char *call, int cy, int a, int x, int y, uint8_t *name, uint8_t *block, int block_len) {
   int i;
   printf("%s: ", call);
   if (cy >= 0) {
      printf("Cy=%02x ", cy);
   }
   if (a >= 0) {
      printf("A=%02x ", a);
   }
   if (x >= 0) {
      printf("X=%02x ", x);
   }
   if (y >= 0) {
      printf("Y=%02x ", y);
   }
   if (name) {
      printf("STRING=%s ", name);
   }
   if (block && block_len > 0) {
      printf("BLOCK=");
      for (i = 0; i < block_len; i++) {
         printf("%02x ", block[i]);
      }
   }
   printf("\n");
}

void r2_p2h_state_machine(uint8_t data) {
   static int a = -1;
   static int x = -1;
   static int y = -1;
   static int in_length = -1;
   static int out_length = -1;
   static int state = R2_IDLE;
   static uint8_t buffer[512];
   static int index = 0;

#ifdef DEBUG
   printf("tube write:  R2 = %02x (state = %d)\n", data, state);
#endif

   // There seems to be a spurious read of R2 by the host
   // during error handling, which we supress here.
   //if (in_error) {
   //   return;
   //}

   // Save the data
   buffer[index] = data;
   if (index < sizeof(buffer) - 1) {
      index++;
   } else {
      printf("Request buffer overflow!, state = %d\n", state);
   }
   switch (state) {
   case R2_IDLE:
      switch (data) {
      case 0x00:
         print_call("R2: OSRDCH", -1,  -1, -1, -1, NULL, NULL, -1);
         do_osrdch();
         break;
      case 0x02:
         state = R2_OSCLI_0;
         break;
      case 0x04:
         state = R2_OSBYTELO_0;
         break;
      case 0x06:
         state = R2_OSBYTEHI_0;
         break;
      case 0x08:
         state = R2_OSWORD_0;
         break;
      case 0x0A:
         state = R2_OSWORD0_0;
         break;
      case 0x0C:
         state = R2_OSARGS_0;
         break;
      case 0x0E:
         state = R2_OSBGET_0;
         break;
      case 0x10:
         state = R2_OSBPUT_0;
         break;
      case 0x12:
         state = R2_OSFIND_0;
         break;
      case 0x14:
         state = R2_OSFILE_0;
         break;
      case 0x16:
         state = R2_OSGBPB_0;
         break;
      default:
         printf("Illegal R2 tube command %02x\n", data);
      }
      break;


   case R2_OSCLI_0:
      if (data == 0x0D) {
         buffer[index - 1] = 0;
         print_call("R2: OSCLI", -1, -1, -1, -1, buffer + 1, NULL, -1);
         do_oscli(buffer + 1);
         state = R2_IDLE;
      }
      break;

   case R2_OSBYTELO_0:
      x = data;
      state = R2_OSBYTELO_1;
      break;
   case R2_OSBYTELO_1:
      a = data;
      print_call("R2: OSBYTE", -1, a, x, -1, NULL, NULL, -1);
      state = R2_IDLE;
      do_osbytelo(a, x);
      break;

   case R2_OSBYTEHI_0:
      x = data;
      state = R2_OSBYTEHI_1;
      break;
   case R2_OSBYTEHI_1:
      y = data;
      state = R2_OSBYTEHI_2;
      break;
   case R2_OSBYTEHI_2:
      a = data;
      print_call("R2: OSBYTE", -1, a, x, y, NULL, NULL, -1);
      do_osbytehi(a, x, y);
      state = R2_IDLE;
      break;


   case R2_OSWORD_0:
      a = data;
      state = R2_OSWORD_1;
      break;
   case R2_OSWORD_1:
      in_length = data;
      if (in_length > 0) {
         state = R2_OSWORD_2;
      } else {
         state = R2_OSWORD_3;
      }
      break;
   case R2_OSWORD_2:
      if (index == in_length + 3) {
         state = R2_OSWORD_3;
      }
      break;
   case R2_OSWORD_3:
      out_length = data;
      print_call("R2: OSWORD", -1, a, -1, -1, NULL, buffer + 3, in_length);
      do_osword(a, buffer + 3, in_length, out_length);
      state = R2_IDLE;
      break;


   case R2_OSWORD0_0:
      state = R2_OSWORD0_1;
      break;
   case R2_OSWORD0_1:
      state = R2_OSWORD0_2;
      break;
   case R2_OSWORD0_2:
      state = R2_OSWORD0_3;
      break;
   case R2_OSWORD0_3:
      state = R2_OSWORD0_4;
      break;
   case R2_OSWORD0_4:
      print_call("R2: OSWORD0", -1, -1, -1, -1, NULL, buffer + 1, 5);
      do_osword0(buffer + 1);
      state = R2_IDLE;
      break;

   case R2_OSARGS_0:
      y = data;
      state = R2_OSARGS_1;
      break;
   case R2_OSARGS_1:
      if (index == 6) {
         // &0C Y <4-byte block> A
         state = R2_OSARGS_2;
      }
      break;
   case R2_OSARGS_2:
      a = data;
      state = R2_IDLE;
      print_call("R2: OSARGS", -1, a, -1, y, NULL, buffer + 2, 5);
      do_osargs(a, y, buffer + 2);
      break;

   case R2_OSBGET_0:
      y = data;
      print_call("R2: OSBGET", -1, -1, -1, y, NULL, NULL, -1);
      state = R2_IDLE;
      do_osbget(y);
      break;

   case R2_OSBPUT_0:
      y = data;
      state = R2_OSBPUT_1;
      break;
   case R2_OSBPUT_1:
      a = data;
      print_call("R2: OSBPUT", a, -1, -1, y, NULL, NULL, -1);
      state = R2_IDLE;
      do_osbput(a, y);
      break;


   case R2_OSFIND_0:
      // OSFIND   R2: &12 &00 Y
      // OSFIND   R2: &12 A string &0D
      a = data;
      if (a == 0) {
         state = R2_OSFIND_1;
      } else {
         state = R2_OSFIND_2;
      }
      break;
   case R2_OSFIND_1:
      y = data;
      print_call("R2: OSFIND", -1,  a, -1, y, NULL, NULL, -1);
      state = R2_IDLE;
      do_osfind0(y);
      break;
   case R2_OSFIND_2:
      if (data == 0x0d) {
         buffer[index - 1] = 0;
         print_call("R2: OSFIND", -1, a, -1, -1, buffer + 2, NULL, -1);
         state = R2_IDLE;
         do_osfindA(a, buffer + 2);
      }
      break;

   case R2_OSFILE_0:
      // OSFILE   R2: &14 block string &0D A
      if (index == 17) {
         state = R2_OSFILE_1;
      }
      break;
   case R2_OSFILE_1:
      if (data == 0x0d) {
         buffer[index - 1] = 0;
         state = R2_OSFIND_2;
      }
      break;
   case R2_OSFILE_2:
      a = data;
      print_call("R2: OSFILE", -1, a, -1, -1, buffer + 16, buffer + 1, 16);
      do_osfile(a, buffer + 1, buffer + 16);
      state = R2_IDLE;
      break;


   case R2_OSGBPB_0:
      if (index == 17) {
         state = R2_OSGBPB_1;
      }
      break;
   case R2_OSGBPB_1:
      printf("R2: OSGBPB not yet implemented\n");
      do_osgbpb(data, buffer + 1);
      state = R2_IDLE;
      break;

   default:
      break;
   }
   if (state == R2_IDLE) {
      index = 0;
   }
}


void tube_write(int reg, uint8_t data) {
   if (reg == 1) {
      do_oswrch(data);
   }
   if (reg == 3) {
      r2_p2h_state_machine(data);
   }
}

uint8_t tube_read(int reg) {
   uint8_t data = 0xEE;
   fifo_type *fifo = NULL;
   switch ((reg >> 1) & 3) {
   case 0:
      fifo = &r1;
      break;
   case 1:
      fifo = &r2;
      break;
   case 2:
      fifo = &r3;
      break;
   case 3:
      fifo = &r4;
      break;
   }
   if (reg & 1) {
      // Read from FIFO
      data = fifo_read(fifo);
   } else {
      // Read FIFO Status
      data = fifo_empty(fifo) ? 0x40 : 0xC0;
   }
   return data;
}
