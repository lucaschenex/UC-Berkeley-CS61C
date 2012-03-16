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

void transpose(int n, float *src, float *dst) {
    const int blocksize = 400;
    int i,j,k,m;
    for (i = 0; i < n; i += blocksize)
        for (j = 0; j < n; j += blocksize)
            for (k = i; (k < i + blocksize) && (k < n); k++)
                for (m = j; (m < j + blocksize) && (m < n); m++)
                    dst[m + k*n] = src[k + m*n];
}

/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float *A, float *B, float *C)
{
    const int blocksize = 4, stride = 16;
    int i, j, k, i2, j2;
    __m128 mmA, mmB, mmC, mmSum1, mmSum2, mmSum3, mmSum4;
    
    float *AT = malloc(n * n * sizeof(float));
    transpose(n, A, AT);
    
    float buffer[4] = {0};
    
    for (i = 0; i < n; i += blocksize) {
        for (j = 0; j < n; j += blocksize) {
            for (i2 = i; (i2 < i + blocksize) && (i2 < n); i2++) {
                for (j2 = j; (j2 < j + blocksize) && (j2 < n); j2++) {
                    mmSum1 = _mm_setzero_ps();
                    mmSum2 = _mm_setzero_ps();
                    mmSum3 = _mm_setzero_ps();
                    mmSum4 = _mm_setzero_ps();
                    for (k = 0; k < (n / stride) * stride; k += stride) {
                        mmA = _mm_loadu_ps(AT + i2*n + k);
                        mmB = _mm_loadu_ps(B + k + j2*n);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum1 = _mm_add_ps(mmSum1, mmA);
                    
                        mmA = _mm_loadu_ps(AT + i2*n + k + 4);
                        mmB = _mm_loadu_ps(B + k + j2*n + 4);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum2 = _mm_add_ps(mmSum2, mmA);
                    
                        mmA = _mm_loadu_ps(AT + i2*n + k + 8);
                        mmB = _mm_loadu_ps(B + k + j2*n + 8);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum3 = _mm_add_ps(mmSum3, mmA);
                    
                        mmA = _mm_loadu_ps(AT + i2*n + k + 12);
                        mmB = _mm_loadu_ps(B + k + j2*n + 12);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum4 = _mm_add_ps(mmSum4, mmA);
                    }    
                    mmSum1 = _mm_add_ps(mmSum1, mmSum2);
                    mmSum3 = _mm_add_ps(mmSum3, mmSum4);
                    mmSum1 = _mm_add_ps(mmSum1, mmSum3);
                    _mm_storeu_ps(buffer, mmSum1);
                    C[i2 + j2*n] += buffer[0] + buffer[1] + buffer[2] + buffer[3];
                    for (k = (n / stride * stride); k < n; k++) {
                        C[i2 + j2*n] += AT[i2*n + k] * B[k + j2*n];
                    }
                }
            }
        }
    }
    free(AT);
}

