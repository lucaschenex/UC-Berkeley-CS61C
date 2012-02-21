typedef long (*cmp_t) (T,T);

long cmp(T a, T b)
{
  if(a < b)
    return -1;
  if(a > b)
    return 1;
  return 0;
}

void __attribute__((noinline))
quicksort(T* base, long n, cmp_t cmp)
{
  long i,j,t;
  T key;
  T tmp;

  if(n < 2)
    return;

  key = base[n/2];
  base[n/2] = base[0];
  base[0] = key;

  i = 1;
  j = n-1;

  while(i <= j)
  {
    while(i != n && (((t = cmp(base[i],key)) >> (8*sizeof(long)-1)) == -1 || t == 0))
    //while(i < n && cmp(base[i],key) <= 0)
      i++;
    while(j != -1 && (((t = cmp(base[j],key)) >> (8*sizeof(long)-1)) != -1 && t != 0))
    //while(j >= 0 && cmp(base[j],key) > 0)
      j--;
    if(((i-j) >> (8*sizeof(long)-1)) == -1)
    //if(i < j)
    {
      tmp = base[i];
      base[i] = base[j];
      base[j] = tmp;
    }
  }

  base[0] = base[j];
  base[j] = key;

  quicksort(base,j,cmp);
  quicksort(base+j+1,n-(j+1),cmp);
}
