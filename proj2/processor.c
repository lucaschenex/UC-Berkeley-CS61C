#include <stdio.h>
#include <stdlib.h>

#include "processor.h"
#include "disassemble.h"

void execute_one_inst(processor_t* p, int prompt, int print_regs)
{
  inst_t inst;

  /* fetch an instruction */
  inst.bits = load_mem(p->pc, SIZE_WORD);

  /* interactive-mode prompt */
  if(prompt)
  {
    if (prompt == 1) {
      printf("simulator paused, enter to continue...");
      while(getchar() != '\n')
        ;
    }
    printf("%08x: ",p->pc);
    disassemble(inst);
  }

  switch (inst.rtype.opcode) /* could also use e.g. inst.itype.opcode */
  {
  case 0x0: // opcode == 0x0 (SPECIAL)

    switch (inst.rtype.funct)
    {
		
    case 0x0: // funct == 0x0 (sll)
	//finish
	p->pc += 4;
	break;
	
    case 0x2: // funct == 0x2 (srl)
	//finish
	p->pc += 4;
	break;

    case 0x3: // funct == 0x3 (sra)
	//finish
	p->pc += 4;
	break;

    case 0x8: // funct == 0x8 (jr)
	//finish
	p->pc += 4;
	break;

    case 0x9: // funct == 0x9 (jalr)
	//finish
	p->pc += 4;
	break;	
	
    case 0xc: // funct == 0xc (SYSCALL)
      handle_syscall(p);
      p->pc += 4;
      break;

    case 0x21: // funct == 0x21 (addu)
	//finish
	p->pc += 4;
	break;

    case 0x23: // funct == 0x23 (subu)
	//finish
	p->pc += 4;
	break;

    case 0x24: // funct == 0x24 (and)
	//finish
	p->pc += 4;
	break;
      
    case 0x25: // funct == 0x25 (OR)
      p->R[inst.rtype.rd] = p->R[inst.rtype.rs] | p->R[inst.rtype.rt];
      p->pc += 4;
      break;

    case 0x26: // funct == 0x26 (xor)
	//finish
	p->pc += 4;
	break;

    case 0x27: // funct == 0x27 (nor)
	//finish
	p->pc += 4;
	break;

    case 0x2a: // funct == 0x2a (slt)
	//finish
	p->pc += 4;
	break;

    case 0x2b: // funct == 0x2b (sltu)
	//finish
	p->pc += 4;
	break;

    default: // undefined funct
      fprintf(stderr, "%s: pc=%08x, illegal instruction=%08x\n", __FUNCTION__, p->pc, inst.bits);
      exit(-1);
      break;
    }
    break;
    
  case 0x2: // opcode == 0x2 (J)
    p->pc = ((p->pc+4) & 0xf0000000) | (inst.jtype.addr << 2);
    break;

  case 0x3: // opcode == 0x3 (jal)
      //finish
      p->pc += 4;
      break;

  case 0x4: // opcode == 0x4 (beq)
      //finish
      p->pc += 4;
      break;

  case 0x5: // opcode == 0x5 (bne)
      //finish
      p->pc += 4;
      break;

  case 0x9: // opcode == 0x9 (addiu)
      //finish
      p->pc += 4;
      break;

  case 0xa: // opcode == 0xa (slti)
      //finish
      p->pc += 4;
      break;

  case 0xb: // opcode == 0xb (sltiu)
      //finish
      p->pc += 4;
      break;

  case 0xc: // opcode == 0xc (andi)
      //finish
      p->pc += 4;
      break;

  case 0xd: // opcode == 0xd (ORI)
    p->R[inst.itype.rt] = p->R[inst.itype.rs] | inst.itype.imm;
    p->pc += 4;
    break;

  case 0xe: // opcode == 0xe (xori)
      //finish
      p->pc += 4;
      break;

  case 0xf: // opcode == 0xf (lui)
      //finish
      p->pc += 4;
      break;

  case 0x20: // opcode == 0x20 (lb)
      //finish
      p->pc += 4;
      break;

  case 0x21: // opcode == 0x21 (lh)
      //finish
      p->pc += 4;
      break;

  case 0x23: // opcode == 0x23 (lw)
      //finish
      p->pc += 4;
      break;

  case 0x24: // opcode == 0x24 (lbu)
      //finish
      p->pc += 4;
      break;

  case 0x25: // opcode == 0x25 (lhu)
      //finish
      p->pc += 4;
      break;

  case 0x28: // opcode == 0x28 (sb)
      //finish
      p->pc += 4;
      break;

  case 0x29: // opcode == 0x29 (sh)
      //finish
      p->pc += 4;
      break;
      
  case 0x2b: // opcode == 0x2b (sw)
      //finish
      p->pc += 4;
      break;
     
  default: // undefined opcode
    fprintf(stderr, "%s: pc=%08x, illegal instruction: %08x\n", __FUNCTION__, p->pc, inst.bits);
    exit(-1);
    break;
  }

  // enforce $0 being hard-wired to 0
  p->R[0] = 0;

  if(print_regs)
    print_registers(p);
}

void init_processor(processor_t* p)
{
  int i;

  /* initialize pc to 0x1000 */
  p->pc = 0x1000;

  /* zero out all registers */
  for (i=0; i<32; i++)
  {
    p->R[i] = 0;
  }
}

void print_registers(processor_t* p)
{
  int i,j;
  for (i=0; i<8; i++)
  {
    for (j=0; j<4; j++)
      printf("r%2d=%08x ",i*4+j,p->R[i*4+j]);
    puts("");
  }
}

void handle_syscall(processor_t* p)
{
  reg_t i;

  switch (p->R[2]) // syscall number is given by $v0 ($2)
  {
  case 1: // print an integer
    printf("%d", p->R[4]);
    break;

  case 4: // print a string
    for(i = p->R[4]; i < MEM_SIZE && load_mem(i, SIZE_BYTE); i++)
      printf("%c", load_mem(i, SIZE_BYTE));
    break;

  case 10: // exit
    printf("exiting the simulator\n");
    exit(0);
    break;

  case 11: // print a character
    printf("%c", p->R[4]);
    break;

  default: // undefined syscall
    fprintf(stderr, "%s: illegal syscall number %d\n", __FUNCTION__, p->R[2]);
    exit(-1);
    break;
  }
}
