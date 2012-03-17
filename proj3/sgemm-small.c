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

void transpose(int n, int padded_size, float *src, float *dst) {
    const int blocksize = 400;
    int i,j,k,m;
    for (i = 0; i < n; i += blocksize)
        for (j = 0; j < n; j += blocksize)
            for (k = i; (k < i + blocksize) && (k < n); k++)
                for (m = j; (m < j + blocksize) && (m < n); m++)
                    dst[m + k*padded_size] = src[k + m*n];
}

/* Pads A to safely ignore matrix sizes. */
void pad(int n, int padded_size, float *A, float *dst) {
        const int blocksize = 400;
        for (int i = 0; i < n; i += blocksize)
            for (int j = 0; j < n; j += blocksize)
                for (int k = i; (k < (i + blocksize)) && (k < n); k++)
                    for (int m = j; (m < (j + blocksize)) && (m < n); m++) {
                        dst[m + k*padded_size] = A[m + k*n];
                    }
}

/* Unpads C to restore matrix sizes. intrinsics? */
void unpad(int n, int padded_size, float *cTmp, float *dst) {
        const int blocksize = 400;
        for (int i = 0; i < n; i += blocksize)
            for (int j = 0; j < n; j += blocksize)
                for (int k = i; (k < (i + blocksize)) && (k < n); k++)
                    for (int m = j; (m < (j + blocksize)) && (m < n); m++) {
                        dst[m + k*n] = cTmp[m + k*padded_size];
                    }
}

/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float *A, float *B, float *C)
{
    const int blocksize = 16, stride = 16;
    int i, j, k, i2, j2;
    __m128 mmA, mmB, mmC, mmSum1, mmSum2, mmSum3, mmSum4;
    
    const int padded_size = (n + blocksize - 1) & ~(blocksize - 1);
    float *C_final = C;
    
    float *AT = calloc(padded_size * padded_size, sizeof(float));
    float *BT = calloc(padded_size * padded_size, sizeof(float));
    C = calloc(padded_size * padded_size, sizeof(float));
    transpose(n, padded_size, A, AT);
    pad(n, padded_size, B, BT);
    B = BT;

    float buffer[4] = {0};
    
    for (i = 0; i < padded_size; i += blocksize) {
        for (j = 0; j < padded_size; j += blocksize) {
            for (i2 = i; i2 < i + blocksize; i2++) {
                for (j2 = j; j2 < j + blocksize; j2++) {
                    float cij = C[i2 + j2*padded_size];
                    mmSum1 = _mm_setzero_ps();
                    mmSum2 = _mm_setzero_ps();
                    mmSum3 = _mm_setzero_ps();
                    mmSum4 = _mm_setzero_ps();
                    for (k = 0; k < padded_size; k += stride) {
                        mmA = _mm_loadu_ps(AT + i2*padded_size + k);
                        mmB = _mm_loadu_ps(B + k + j2*padded_size);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum1 = _mm_add_ps(mmSum1, mmA);
                    
                        mmA = _mm_loadu_ps(AT + i2*padded_size + k + 4);
                        mmB = _mm_loadu_ps(B + k + j2*padded_size + 4);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum2 = _mm_add_ps(mmSum2, mmA);
                    
                        mmA = _mm_loadu_ps(AT + i2*padded_size + k + 8);
                        mmB = _mm_loadu_ps(B + k + j2*padded_size + 8);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum3 = _mm_add_ps(mmSum3, mmA);
                    
                        mmA = _mm_loadu_ps(AT + i2*padded_size + k + 12);
                        mmB = _mm_loadu_ps(B + k + j2*padded_size + 12);
                        mmA = _mm_mul_ps(mmA, mmB);
                        mmSum4 = _mm_add_ps(mmSum4, mmA);
                    }    
                    mmSum1 = _mm_add_ps(mmSum1, mmSum2);
                    mmSum3 = _mm_add_ps(mmSum3, mmSum4);
                    mmSum1 = _mm_add_ps(mmSum1, mmSum3);
                    _mm_storeu_ps(buffer, mmSum1);
                    cij += buffer[0] + buffer[1] + buffer[2] + buffer[3];
                    C[i2 + j2*padded_size] = cij;
                }
            }
        }
    }

    unpad(n, padded_size, C, C_final);

    free(AT);
    free(BT);
    free(C);
}

