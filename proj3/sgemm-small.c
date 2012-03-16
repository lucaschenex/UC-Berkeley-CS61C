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

/* Matrix transpose. */
void transpose(int n, float *src, float *dst) {
    const int blocksize = 100;
    int i,j,k,m;
    for (i = 0; i < n; i += blocksize)
        for (j = 0; j < n; j += blocksize)
            for (m = j; (m < j + blocksize) && (m < n); m++)
                for (k = i; (k < i + blocksize) && (k < n); k++)
                    dst[m + k*n] = src[k + m*n];
}

/* This routine performs a sgemm operation
 * C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float* A, float* B, float* C)
{
    int k, j, k2, j2, i;
    const int blocksize = 16;
    __m128 mmA, mmB, mmC;
    
    //convert B to column major
    float *BT = malloc(n * n * sizeof(float));
    transpose(n, B, BT);
    B = BT;

    for (k = 0; k < n; k += blocksize)
    	for (j = 0; j < n; j += blocksize)
    	    for (k2 = k; (k2 < k + blocksize) && (k2 < n); k2++) {
                for (j2 = j; (j2 < j + blocksize) && (j2 < n); j2++) {
                    float bkj = B[k2 * n + j2];
                    mmB = _mm_load1_ps(&bkj);
                    for (i = 0; i < (n & ~0xf); i += 16) {
                        mmC = _mm_loadu_ps(C + i + j2*n);
                        mmA = _mm_loadu_ps(A + i + k2*n);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmC = _mm_add_ps(mmC, mmA);
                        _mm_storeu_ps(C + i + j2*n, mmC);
                        
                        mmC = _mm_loadu_ps(C + i + j2*n + 4);
                        mmA = _mm_loadu_ps(A + i + k2*n + 4);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmC = _mm_add_ps(mmC, mmA);
                        _mm_storeu_ps(C + i + j2*n + 4, mmC);

                        mmC = _mm_loadu_ps(C + i + j2*n + 8);
                        mmA = _mm_loadu_ps(A + i + k2*n + 8);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmC = _mm_add_ps(mmC, mmA);
                        _mm_storeu_ps(C + i + j2*n + 8, mmC);
                        
                        mmC = _mm_loadu_ps(C + i + j2*n + 12);
                        mmA = _mm_loadu_ps(A + i + k2*n + 12);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmC = _mm_add_ps(mmC, mmA);
                        _mm_storeu_ps(C + i + j2*n + 12, mmC);
                    }

                    for (i = n & ~0xf; i < (n & ~0x3); i += 4) {
                        mmC = _mm_loadu_ps(C + i + j2*n);
                        mmA = _mm_loadu_ps(A + i + k2*n);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmC = _mm_add_ps(mmC, mmA);
                        _mm_storeu_ps(C + i + j2*n, mmC);
                    }

                    for(i = n & ~0x3; i < n; i++)
                        C[i + j2*n] += A[i + k2*n] * bkj; 
    		}
        }

    free(BT);
}

