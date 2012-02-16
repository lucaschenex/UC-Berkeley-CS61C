#include <stdio.h>

int main(int argc, char** argv)
{
  int a = 2;	
  printf("hello, world!\n");
  a = 3;
  child(a);
  return 0;
}

void child(int i) {
    printf("child\n");
}
