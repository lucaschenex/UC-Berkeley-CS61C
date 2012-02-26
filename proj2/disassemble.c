#include <stdio.h>
#include <stdlib.h>

#include "disassemble.h"

void disassemble(inst_t inst)
{
  /* you could use itype or type instead of rtype */
  /* remember that opcode field is at the same place for all types */
  switch (inst.rtype.opcode)
  {
  case 0x0: // opcode == 0x0 (SPECIAL)

    switch (inst.rtype.funct)
    {	
    case 0x0: // funct == 0x0 (sll)
	printf("sll\t$%d,$%d,%u\n", inst.rtype.rd, inst.rtype.rt, inst.rtype.shamt);
	break;
	
    case 0x2: // funct == 0x2 (srl)
	printf("srl\t$%d,$%d,%u\n", inst.rtype.rd, inst.rtype.rt, inst.rtype.shamt);
	break;
	
    case 0x3: // funct == 0x3 (sra)
	printf("sra\t$%d,$%d,%u\n", inst.rtype.rd, inst.rtype.rt, inst.rtype.shamt);
	break;

    case 0x8: // funct == 0x8 (jr)
	printf("jr\t$%d\n", inst.rtype.rs);
	break;

    case 0x9: // funct == 0x9 (jalr)
	printf("jalr\t$%d,%d\n", inst.rtype.rd, inst.rtype.rs);
	break;
	
    case 0xc: // funct == 0xc (SYSCALL)
      printf("syscall\n");
      break;

    case 0x21: // funct == 0x21 (addu)
	printf("addu\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
	break;

    case 0x23: // funct == 0x23 (subu)
	printf("subu\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
	break;

    case 0x24: // funct == 0x24 (and)
	printf("and\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
	break;
      
    case 0x25: // funct == 0x25 (OR)
      printf("or\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
      break;

    case 0x26: // funct == 0x26 (xor)
	printf("xor\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
	break;

    case 0x27: // funct == 0x27 (nor)
	printf("nor\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
	break;

    case 0x2a: // funct == 0x2a (slt)
	printf("slt\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
	break;

    case 0x2b: // funct == 0x2b (sltu)
	printf("sltu\t$%d,$%d,$%d\n", inst.rtype.rd, inst.rtype.rs, inst.rtype.rt);
	break;

    default: // undefined funct
      fprintf(stderr, "%s: illegal instruction: %08x\n", __FUNCTION__, inst.bits);
      exit(-1);
      break;
    }
    break;
    
  case 0x2: // opcode == 0x2 (J)
    printf("j\t0x%x\n", inst.jtype.addr*4);
    break;

  case 0x3: // opcode == 0x3 (jal)
      printf("jal\t0x%x\n", 4 * inst.jtype.addr);
      break;

  case 0x4: // opcode == 0x4 (beq)
      printf("beq\t$%d,$%d,%d\n", inst.itype.rs, inst.itype.rt, 4 * inst.itype.imm);
      break;

  case 0x5: // opcode == 0x5 (bne)
      printf("bne\t$%d,$%d,%d\n", inst.itype.rs, inst.itype.rt, 4 * inst.itype.imm);
      break;

  case 0x9: // opcode == 0x9 (addiu)
      printf("addiu\t$%d,$%d,%d\n", inst.itype.rt, inst.itype.rs, inst.itype.imm);
      break;

  case 0xa: // opcode == 0xa (slti)
      printf("slti\t$%d,$%d,%d\n", inst.itype.rt, inst.itype.rs, inst.itype.imm);
      break;

  case 0xb: // opcode == 0xb (sltiu)
      printf("sltiu\t$%d,$%d,%d\n", inst.itype.rt, inst.itype.rs, inst.itype.imm);
      break;

  case 0xc: // opcode == 0xc (andi)
      printf("andi\t$%d,$%d,0x%x\n", inst.itype.rt, inst.itype.rs, inst.itype.imm);
      break;

  case 0xd: // opcode == 0xd (ORI)
    printf("ori\t$%d,$%d,0x%x\n", inst.itype.rt, inst.itype.rs, inst.itype.imm);
    break;

  case 0xe: // opcode == 0xe (xori)
      printf("xori\t$%d,$%d,0x%x\n", inst.itype.rt, inst.itype.rs, inst.itype.imm);
      break;

  case 0xf: // opcode == 0xf (lui)
      printf("lui\t$%d,0x%x\n", inst.itype.rt, inst.itype.imm);
      break;

  case 0x20: // opcode == 0x20 (lb)
      printf("lb\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;

  case 0x21: // opcode == 0x21 (lh)
      printf("lh\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;

  case 0x23: // opcode == 0x23 (lw)
      printf("lw\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;

  case 0x24: // opcode == 0x24 (lbu)
      printf("lbu\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;

  case 0x25: // opcode == 0x25 (lhu)
      printf("lhu\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;

  case 0x28: // opcode == 0x28 (sb)
      printf("sb\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;

  case 0x29: // opcode == 0x29 (sh)
      printf("sh\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;
      
  case 0x2b: // opcode == 0x2b (sw)
      printf("sw\t$%d,%u($%d)\n", inst.itype.rt, 4 * inst.itype.imm, inst.itype.rs);
      break;
    
  default: // undefined opcode
    fprintf(stderr, "%s: illegal instruction: %08x\n", __FUNCTION__, inst.bits);
    exit(-1);
    break;
  }
}
