#include <stdlib.h>
#include <stdio.h>
#include <cblas.h>

void square_sgemm (int n, float* A, float* B, float* C)
{
  cblas_sgemm( CblasColMajor,CblasNoTrans,CblasNoTrans, n,n,n, 1, A,n, B,n, 1, C,n );
}
