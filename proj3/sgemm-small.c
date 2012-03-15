/*
  CS 61C Project3

  Name: Hairan Zhu
  Login: cs61c-eu

  Name: Benjamin Han
  Login: cs61c-mm
 */

#include <emmintrin.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <cblas.h>

#define TSC_PER_SECOND (2400*1e6)
static __inline__ unsigned long long rdtsc(void)
{
  unsigned hi, lo;
  __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
  return ( (unsigned long long)lo)|( ((unsigned long long)hi)<<32 );
}

/* Converts matrix B to column-major to reduce stride */
void transpose(int n, int blocksize, float *B) {
    float tmp; int i,j,k,m;
    for (i = 0; i < n; i += blocksize) 
	for (j = 0; j < n; j += blocksize) 
	    for (k = i; (k < i + blocksize) & (k < n) ; k++ ) 
		for (m = j; (m < j + blocksize) & (m < k); m++ ) {
		    tmp = B[k + m*n];
		    B[m + k*n] = B[k + m*n];
		    B[m + k*n] = tmp;
		}
}

/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float* A, float* B, float* C)
{
    int i, j, k, i2, j2, blocksize = 16, t_blocksize = 400;
    __m128 mmA, mmB, mmTmp1, mmTmp2, mmTmp3, mmTmp4;
    //convert A to row major
    transpose(n, t_blocksize, A);

    float buffer[4] = {0};

    for (i = 0; i < n; i += blocksize)
    for (j = 0; j < n; j += blocksize)
    for (i2 = i; (i2 < i + blocksize) && (i2 < n); i2++)
	for (j2 = j; (j2 < j + blocksize) && (j2 < n); j2++) {
        mmTmp1 = _mm_setzero_ps();
        mmTmp2 = _mm_setzero_ps();
        mmTmp3 = _mm_setzero_ps();
        mmTmp4 = _mm_setzero_ps();
        for (k = 0; k < (n / blocksize) * blocksize; k += blocksize) {
            mmA = _mm_loadu_ps(A + i2*n + k);
            mmB = _mm_loadu_ps(B + k + j2*n);
            mmTmp1 = _mm_add_ps(mmTmp1, _mm_mul_ps(mmA, mmB));
            mmA = _mm_loadu_ps(A + i2*n + k + 4);
            mmB = _mm_loadu_ps(B + k + j2*n + 4);
            mmTmp2 = _mm_add_ps(mmTmp2, _mm_mul_ps(mmA, mmB));
            mmA = _mm_loadu_ps(A + i2*n + k + 8);
            mmB = _mm_loadu_ps(B + k + j2*n + 8);
            mmTmp3 = _mm_add_ps(mmTmp3, _mm_mul_ps(mmA, mmB));
            mmA = _mm_loadu_ps(A + i2*n + k + 12);
            mmB = _mm_loadu_ps(B + k + j2*n + 12);
            mmTmp4 = _mm_add_ps(mmTmp4, _mm_mul_ps(mmA, mmB));
        }
        mmTmp1 = _mm_add_ps(mmTmp1, mmTmp2);
        mmTmp3 = _mm_add_ps(mmTmp3, mmTmp4);
        mmTmp1 = _mm_add_ps(mmTmp1, mmTmp3);
        _mm_storeu_ps(buffer, mmTmp1);
        C[i2 + j2*n] += buffer[0] + buffer[1] + buffer[2] + buffer[3];
	    for (k = (n / blocksize * blocksize); k < n; k++)
            C[i2 + j2*n] += A[i2 + k*n] * B[k + j2*n];
    }
}

