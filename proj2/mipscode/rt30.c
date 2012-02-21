#ifndef __x86_64__
#include "io.h"
#endif

#define strsort
#ifdef strsort
#define T const char*
#else
#define T int
#endif
#include "qsort.c"

void print(T* a, unsigned long n)
{
  long i;
  for(i = 0; i < n; i++)
  {
    #ifdef strsort
    mips_print_string(a[i]);
    #else
    mips_print_int(a[i]);
    #endif
    mips_print_char(' ');
  }
  mips_print_char('\n');
}

#ifdef strsort
long stringcmp(T a, T b)
{
  int i;
  for(i = 0; a[i] && b[i]; i++)
    if(a[i] != b[i])
      return a[i]-b[i];
  return a[i] != b[i] ? a[i]-b[i] : 0;
}
#endif

int main()
{
  #ifdef strsort
  #define N 5
  T a[N] = { "cat", "ca\xFF", "dog", "scott", "henry" };
  #else
  #define N 25
  T a[N] = {-43, -1, 23, 8, -20, 22, -6, 28, -27, -41, -10, 15, 42, -8, 37, -47, -23, -21, -10, -38, -47, 19, -41, 7, 10};
  #endif

  print(a,N);
  #ifdef strsort
  quicksort(a,N,stringcmp);
  #else
  quicksort(a,N,cmp);
  #endif
  print(a,N);

  return 0;
}
