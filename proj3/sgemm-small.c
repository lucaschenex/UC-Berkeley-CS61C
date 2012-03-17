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

/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float *A, float *B, float *C)
{
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

    free(Acpy);
    free(Bcpy);
    free(Ccpy);
}

