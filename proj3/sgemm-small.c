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
    for ( i = 0; i < n; i += blocksize ) 
	for ( j = 0; j < n; j += blocksize ) 
	    for ( k = i; (k < i + blocksize) & (k < n) ; k++ ) 
		for ( m = j; (m < j + blocksize) & (m < n); m++ ) {
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
    float tmpA[n]; float tmpC[n];
    int k, j, k2, j2, i; int blocksize = 36;
    __m128 mmA; __m128 mmB; __m128 mmC; __m128 mmTmp;
    
    //convert B to column major
    transpose(n, blocksize, B);
    
    for (k = 0; k < n; k += blocksize)
    	for (j = 0; j < n; j += blocksize)
    	    for (k2 = k; (k2 < k + blocksize) & (k2 < n); k2++) {
		for (j2 = j; (j2 < j + blocksize) & (j2 < n); j2++) {
		    mmB = _mm_load1_ps(B + k2*n + j2);
    		    for (i = 0; i < n/4*4; i += 4) {
    			mmA = _mm_loadu_ps(A + i + k2*n);
    			mmC = _mm_loadu_ps(C + i + j2*n);
    			mmTmp = _mm_add_ps(mmC, _mm_mul_ps(mmA, mmB));
			_mm_storeu_ps(C + i + j2*n, mmTmp);
    		    }
		    for( int i = n/4*4; i < n; i++ )
			C[i + j2*n] += A[i + k2*n] * B[k2*n + j2];
    		}
    	    }
}
