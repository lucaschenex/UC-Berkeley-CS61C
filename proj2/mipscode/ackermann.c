#include "io.h"

// this function computes the Ackermann function A(m,n).
// A(0,n) = n + 1
// A(m,0) = A(m-1,1) for m > 0
// A(m,n) = A(m-1,A(m,n-1)) for m > 0 and n > 0
int ackermann(int m, int n)
{
  if(m == 0)
    return n+1;
  if(n == 0)
    return ackermann(m-1, 1);
  return ackermann(m-1, ackermann(m, n-1));
}

int main()
{
  int m = 3, n = 3;

  mips_print_string("A(");
  mips_print_int(m);
  mips_print_string(", ");
  mips_print_int(n);
  mips_print_string(") = ");
  mips_print_int(ackermann(m,n));
  mips_print_string("\n");

  return 0;
}
