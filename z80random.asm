org 0x0000
   JP INIT
   DS 5

   RET
   DS 7
   RET
   DS 7
   RET
   DS 7
   RET
   DS 7
   RET
   DS 7
   RET
   DS 7
   RET
   DS 7

   DS 0x26

NMI:
   JP NMI1
NMI1:
   LD (0x00FE), SP
   LD SP, 0x7FFF
   PUSH IX
   PUSH IY
   PUSH AF
   PUSH BC
   PUSH DE
   PUSH HL
   EXX
   EX AF,AF'
   PUSH AF
   PUSH BC
   PUSH DE
   PUSH HL
   HALT

INIT:
   IM 0
   EI
   LD A, 0
   LD I, A
   LD R, A
   LD SP, 0x7FFF
   LD IX, 0x8FFF
   LD IY, 0x9FFF
   LD HL, 0x0000
   LD BC, 0x1111
   LD DE, 0x2222
   PUSH HL
   POP AF
   EXX
   EX AF,AF'
   LD HL, 0x3333
   LD BC, 0x4444
   LD DE, 0x5555
   PUSH HL
   POP AF
   EXX
   EX AF,AF'
   JP start

org 0x0100

start:
