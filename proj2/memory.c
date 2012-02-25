#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory.h"

/* Pointer to simulator memory */
uint8_t *mem;

/* Called by program loader to initialize memory. */
uint8_t *init_mem() {
  assert (mem == NULL);
  mem = calloc(MEM_SIZE, sizeof(uint8_t)); // allocate zeroed memory
  return mem;
}

/* Returns 1 if memory access is ok, otherwise 0 */
int access_ok(uint32_t mipsaddr, mem_unit_t size) {

  /* TODO YOUR CODE HERE */
    return ((mipsaddr % size) == 0);
    
}

/* Writes size bytes of value into mips memory at mipsaddr */
void store_mem(uint32_t mipsaddr, mem_unit_t size, uint32_t value) {
  if (!access_ok(mipsaddr, size)) {
    fprintf(stderr, "%s: bad write=%08x\n", __FUNCTION__, mipsaddr);
    exit(-1);
  }

  /* TODO YOUR CODE HERE */
  //int i; int j;
  //for (i = 0, j = mipsaddr; i < size; i++, j++) {
  if (size == 4) {
      *(mem + mipsaddr) = value;
  } else if (size == 2) {
      *(mem + mipsaddr) = value;
  } else {
      *(mem + mipsaddr) = value;
  }

}

/* Returns zero-extended value from mips memory */
uint32_t load_mem(uint32_t mipsaddr, mem_unit_t size) {
  if (!access_ok(mipsaddr, size)) {
    fprintf(stderr, "%s: bad read=%08x\n", __FUNCTION__, mipsaddr);
    exit(-1);
  }

  /* TODO YOUR CODE HERE */
  if (size == 4) {
      return *(uint32_t*)(mem + mipsaddr);
  } else if (size == 2) {
      return *(uint8_t*)(mem + mipsaddr);
  } else {
      return *(mem + mipsaddr);
  }
  
  // incomplete stub to let mipscode/simple execute
  // (only handles size == SIZE_WORD correctly)
  // feel free to delete and implement your own way
  //return *(uint32_t*)(mem + mipsaddr);
}
