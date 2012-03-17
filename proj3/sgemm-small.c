/*
  CS 61C Project3

  Name: Hairan Zhu
  Login: cs61c-eu

  Name: Benjamin Han
  Login: cs61c-mm
 */

#include <emmintrin.h>
#include <pmmintrin.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Transpose with padding. */
void transpose(int n, int padded_size, float *src, float *dst) {
    const int blocksize = 200;
    for (int i = 0; i < n; i += blocksize)
        for (int j = 0; j < n; j += blocksize)
            for (int i2 = i; (i2 < i + blocksize) && (i2 < n); i2++)
                for (int j2 = j; (j2 < j + blocksize) && (j2 < n); j2++)
                    dst[i2 + j2*padded_size] = src[j2 + i2*n];
}

/* Pads A to safely ignore matrix sizes. */
void pad(int n, int padded_size, float *A, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*padded_size, A + i*n, n * sizeof(float));
}

/* Unpads C to restore matrix sizes. intrinsics? */
void unpad(int n, int padded_size, float *cTmp, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*n, cTmp + i*padded_size, n * sizeof(float));
}

/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float *A, float *B, float *C)
{
    const int stride = 16;
    int outer_stride;
    if (n <= 64) {
        outer_stride = 16;
    } else if (n <= 256) {
        outer_stride = 8;
    } else if (n <= 512) {
        outer_stride = 4; 
    } else {
        outer_stride = 2;
    }

    __m128 mmA1, mmA2, mmB1, mmB2, mmC1, mmC2, mmSum1, mmSum2;
    
    const int padded_size = (n + stride  - 1) & ~(stride - 1);
    
    float *AT = calloc(padded_size * padded_size, sizeof(float));
    float *Bcpy = calloc(padded_size * padded_size, sizeof(float));
    float* Ccpy = calloc(padded_size * padded_size, sizeof(float));
    
    transpose(n, padded_size, A, AT);
    pad(n, padded_size, B, Bcpy);

    for (int i = 0; i < padded_size; i += stride)
        for (int j = 0; j < padded_size; j += stride)
            for (int i2 = i; i2 < i + stride; i2++)
                for (int j2 = j; j2 < j + stride; j2++) {
                    mmSum1 = _mm_load_ss(Ccpy + i2 + j2*padded_size);
                    mmSum2 = _mm_setzero_ps();
                    for (int k = 0; k < padded_size; k += stride) {
                        mmA1 = _mm_loadu_ps(AT + i2*padded_size + k);
                        mmB1 = _mm_loadu_ps(Bcpy + k + j2*padded_size);
                        mmA1 = _mm_mul_ps(mmA1, mmB1);
                        mmSum1 = _mm_add_ps(mmSum1, mmA1);
                    
                        mmA2 = _mm_loadu_ps(AT + i2*padded_size + k + 4);
                        mmB2 = _mm_loadu_ps(Bcpy + k + j2*padded_size + 4);
                        mmA2 = _mm_mul_ps(mmA2, mmB2);
                        mmSum2 = _mm_add_ps(mmSum2, mmA2);
                    
                        mmA1 = _mm_loadu_ps(AT + i2*padded_size + k + 8);
                        mmB1 = _mm_loadu_ps(Bcpy + k + j2*padded_size + 8);
                        mmA1 = _mm_mul_ps(mmA1, mmB1);
                        mmSum1 = _mm_add_ps(mmSum1, mmA1);
                    
                        mmA2 = _mm_loadu_ps(AT + i2*padded_size + k + 12);
                        mmB2 = _mm_loadu_ps(Bcpy + k + j2*padded_size + 12);
                        mmA2 = _mm_mul_ps(mmA2, mmB2);
                        mmSum2 = _mm_add_ps(mmSum2, mmA2);
                    }
                    mmSum1 = _mm_add_ps(mmSum1, mmSum2);   
                    mmSum1 = _mm_hadd_ps(mmSum1, mmSum1);
                    mmSum1 = _mm_hadd_ps(mmSum1, mmSum1);
                    _mm_store_ss(Ccpy + i2 + j2*padded_size, mmSum1);
                }
    
    unpad(n, padded_size, Ccpy, C);

    free(AT);
    free(Bcpy);
    free(Ccpy);
}

