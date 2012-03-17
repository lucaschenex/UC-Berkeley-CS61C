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





void square64_sgemm (float *A, float *B, float *C);

/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float *A, float *B, float *C)
{
    if (n == 64) {
        square64_sgemm(A, B, C);
        return;
    }

    const int stride = 16;
    const int padded_size = (n + stride - 1) & ~(stride - 1);

    float *Acpy = calloc(padded_size * padded_size, sizeof(float));
    float *Bcpy = calloc(padded_size * padded_size, sizeof(float));
    float *Ccpy = calloc(padded_size * padded_size, sizeof(float));
    pad(n, padded_size, A, Acpy);
    pad(n, padded_size, B, Bcpy);

    for (int k = 0; k < padded_size; k += stride)
        for (int j = 0; j < padded_size; j += stride)
            for (int i = 0; i < padded_size; i += stride)
                for (int k2 = k; k2 < k + stride; k2++)
                    for (int j2 = j; j2 < j + stride; j2++) {
                        float bkj = Bcpy[k2 + j2*padded_size];
                        for (int i2 = i; i2 < i + stride; i2++) {
                            Ccpy[i2 + j2*padded_size] += Acpy[i2 + k2*padded_size] * bkj;   
                        }
                    }

    unpad(n, padded_size, Ccpy, C);
}


/** Hard coded for 64. */
void square64_sgemm (float *A, float *B, float *C)
{
    __m128 mmA1, mmA2, mmB1, mmB2, mmC1, mmC2, mmSum1, mmSum2;
    
    /* Transpose A. */
    float *AT = calloc(64 * 64, sizeof(float));
    for (int i = 0; i < 64; i += 16)
        for (int j = 0; j < 64; j += 16)
            for (int i2 = i; i2 < i + 16; i2++)
                for (int j2 = j; j2 < j + 16; j2++)
                    AT[i2 + j2*64] = A[j2 + i2*64];
    
    for (int i = 0; i < 64; i ++)
        for (int j = 0; j < 64; j ++) {
            mmSum1 = _mm_load_ss(C + i + j*64);
            mmSum2 = _mm_setzero_ps();
            for (int k = 0; k < 64; k += 16) {
                mmA1 = _mm_loadu_ps(AT + i*64 + k);
                mmB1 = _mm_loadu_ps(B + k + j*64);
                mmA1 = _mm_mul_ps(mmA1, mmB1);
                mmSum1 = _mm_add_ps(mmSum1, mmA1);
            
                mmA2 = _mm_loadu_ps(AT + i*64 + k + 4);
                mmB2 = _mm_loadu_ps(B + k + j*64 + 4);
                mmA2 = _mm_mul_ps(mmA2, mmB2);
                mmSum2 = _mm_add_ps(mmSum2, mmA2);
            
                mmA1 = _mm_loadu_ps(AT + i*64 + k + 8);
                mmB1 = _mm_loadu_ps(B + k + j*64 + 8);
                mmA1 = _mm_mul_ps(mmA1, mmB1);
                mmSum1 = _mm_add_ps(mmSum1, mmA1);
            
                mmA2 = _mm_loadu_ps(AT + i*64 + k + 12);
                mmB2 = _mm_loadu_ps(B + k + j*64 + 12);
                mmA2 = _mm_mul_ps(mmA2, mmB2);
                mmSum2 = _mm_add_ps(mmSum2, mmA2);
            }
            mmSum1 = _mm_add_ps(mmSum1, mmSum2);
            mmSum1 = _mm_hadd_ps(mmSum1, mmSum1);
            mmSum1 = _mm_hadd_ps(mmSum1, mmSum1);
            _mm_store_ss(C + i + j*64, mmSum1);
        }
    free(AT);
}

