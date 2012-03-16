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

void transpose(int n, int blocksize, float *src, float *dst) {
    int i,j,k,m;
    for (i = 0; i < n; i += blocksize)
        for (j = 0; j < n; j += blocksize)
            for (k = i; (k < i + blocksize) && (k < n); k++)
                for (m = j; (m < j + blocksize) && (m < n); m++)
                    dst[m + k*n] = src[k + m*n];
}

/*
void square_sgemm(int n, float* A, float* B, float* C) {
    float* AT = malloc(n * n * sizeof(float));
    int i, j, k;
    float cij;
    transpose(n, 300, A, AT);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) { 
            cij = C[i+j*n];
            for (k = 0; k < n; k++)
                cij += AT[k+i*n] * B[k+j*n];
        C[i+j*n] = cij;
        }
    }
    free(AT);
}
*/

/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float *A, float *B, float *C)
{
    const int blocksize = 4, t_blocksize = 400, stride = 8;
    int i, j, k, i2, j2;
    __m128 mmA, mmB, mmC, mmSum;
    
    float *AT = malloc(n * n * sizeof(float));
    transpose(n, t_blocksize, A, AT);
    
    float buffer[4] = {0};
    
    for (i = 0; i < n; i += blocksize) {
        for (j = 0; j < n; j += blocksize) {
            for (i2 = i; (i2 < i + blocksize) && (i2 < n); i2++) {
                for (j2 = j; (j2 < j + blocksize) && (j2 < n); j2++) {
                    mmSum = _mm_setzero_ps();
                    for (k = 0; k < (n / stride) * stride; k += stride) {
                        mmA = _mm_loadu_ps(AT + i2*n + k);
                        mmB = _mm_loadu_ps(B + k + j2*n);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum = _mm_add_ps(mmSum, mmA);
                        mmA = _mm_loadu_ps(AT + i2*n + k + 4);
                        mmB = _mm_loadu_ps(B + k + j2*n + 4);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum = _mm_add_ps(mmSum, mmA);
                    }
                    _mm_storeu_ps(buffer, mmSum);
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

