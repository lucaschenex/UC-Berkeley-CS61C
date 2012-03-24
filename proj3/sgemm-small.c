/*
  CS 61C Project3

  Name: Hairan Zhu
  Login: cs61c-eu

  Name: Benjamin Han
  Login: cs61c-mm
 */

#include <emmintrin.h>
#include <stdlib.h>
#include <string.h>

/* Determines cache blocking (must divide 16 in the general case). */
#define BLOCK 16
#define BLOCK_64 64
#define BLOCK_768 4

/* How much to unroll the i loop. */
#define I_STRIDE 16
/* How much to unroll the k loop. */
#define K_STRIDE 2

/* Forward declaration. */
void squarepad_sgemm(int n, float *A, float *B, float *C);

/* Pads A to safely ignore matrix sizes. */
inline void pad(int n, int padded_size, float *A, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*padded_size, A + i*n, n * sizeof(float));
}

/* Unpads C to restore matrix sizes. */
inline void unpad(int n, int padded_size, float *cTmp, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*n, cTmp + i*padded_size, n * sizeof(float));
}

/* C = C + AB. */
void square_sgemm(int n, float *A, float *B, float *C) {
    if (n % I_STRIDE == 0) {
        squarepad_sgemm(n, A, B, C);
    } else {
        const int npad = ((n + I_STRIDE - 1) / I_STRIDE) * I_STRIDE;
        float *Acpy = calloc(npad * npad, sizeof(float));
        float *Bcpy = calloc(npad * npad, sizeof(float));
        float *Ccpy = calloc(npad * npad, sizeof(float));
        
        pad(n, npad, A, Acpy);
        pad(n, npad, B, Bcpy);
    
        /* Call matrix multiply on padded matrices. */
        squarepad_sgemm(npad, Acpy, Bcpy, Ccpy);

        unpad(n, npad, Ccpy, C);
        free(Acpy);
        free(Bcpy);
        free(Ccpy);
    }
}

/** Assumes input matrix has dimension n divisible by STRIDE. */
inline void squarepad_sgemm (const int n, float *A, float *B, float *C) {
    int j_block;
    if (n == 64)
         j_block = BLOCK_64;
    else if (n < 768)
        j_block = BLOCK;
    else
        j_block = BLOCK_768;
    
    __m128 mmA1, mmA2, mmA3, mmA4, mmA5, mmA6, mmA7, mmA8, mmB1, mmB2, mmC1, mmC2, mmC3, mmC4, mmProd;
 
    for (int k = 0; k < n; k += BLOCK)
    for (int j = 0; j < n; j += j_block)
    for (int k2 = k; k2 < k + BLOCK; k2 += K_STRIDE)
        for (int i = 0; i < n; i += I_STRIDE) {
            mmA1 = _mm_load_ps(A + i + k2*n);
            mmA2 = _mm_load_ps(A + i + k2*n + 4);
            mmA3 = _mm_load_ps(A + i + k2*n + 8);
            mmA4 = _mm_load_ps(A + i + k2*n + 12);
            mmA5 = _mm_load_ps(A + i + (k2 + 1)*n);
            mmA6 = _mm_load_ps(A + i + (k2 + 1)*n + 4);
            mmA7 = _mm_load_ps(A + i + (k2 + 1)*n + 8);
            mmA8 = _mm_load_ps(A + i + (k2 + 1)*n + 12);
            for (int j2 = j; j2 < j + j_block; j2++) {
                mmB1 = _mm_load_ps1(B + k2 + j2*n);
                mmB2 = _mm_load_ps1(B + (k2 + 1) + j2*n);
                
                mmC1 = _mm_load_ps(C + i + j2*n);
                mmProd = _mm_mul_ps(mmA1, mmB1);
                mmC1 = _mm_add_ps(mmProd, mmC1);
                mmProd = _mm_mul_ps(mmA5, mmB2);
                mmC1 = _mm_add_ps(mmProd, mmC1);
                _mm_store_ps(C + i + j2*n, mmC1);

                mmC2 = _mm_load_ps(C + i + j2*n + 4);
                mmProd = _mm_mul_ps(mmA2, mmB1);
                mmC2 = _mm_add_ps(mmProd, mmC2);
                mmProd = _mm_mul_ps(mmA6, mmB2);
                mmC2 = _mm_add_ps(mmProd, mmC2);
                _mm_store_ps(C + i + j2*n + 4, mmC2);
                
                mmC3 = _mm_load_ps(C + i + j2*n + 8);
                mmProd = _mm_mul_ps(mmA3, mmB1);
                mmC3 = _mm_add_ps(mmProd, mmC3);
                mmProd = _mm_mul_ps(mmA7, mmB2);
                mmC3 = _mm_add_ps(mmProd, mmC3);
                _mm_store_ps(C + i + j2*n + 8, mmC3);
                    
                mmC4 = _mm_load_ps(C + i + j2*n + 12);
                mmProd = _mm_mul_ps(mmA4, mmB1);
                mmC4 = _mm_add_ps(mmProd, mmC4);
                mmProd = _mm_mul_ps(mmA8, mmB2);
                mmC4 = _mm_add_ps(mmProd, mmC4);
                _mm_store_ps(C + i + j2*n + 12, mmC4);
            }
        }
}

