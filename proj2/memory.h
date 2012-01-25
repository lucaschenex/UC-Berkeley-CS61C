#ifndef __MIPS_MEM_H
#define __MIPS_MEM_H

#include <stdint.h>

typedef enum {
  SIZE_BYTE = 1,
  SIZE_HALF_WORD = 2,
  SIZE_WORD = 4,
} mem_unit_t;

#define MEM_SIZE (1024*1024) /* 1 Mbyte of memory */

uint8_t *init_mem();
void store_mem(uint32_t mipsaddr, mem_unit_t size, uint32_t value);
uint32_t load_mem(uint32_t mipsaddr, mem_unit_t size);

#endif // __PROCESSOR_H
