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

#define STRIDE 16
#define STRIDE_64 32
#define STRIDE_768 8

/* Forward declaration. */
void squarepad_sgemm(int n, float *A, float *B, float *C);

/* Pads A to safely ignore matrix sizes. */
void pad(int n, int padded_size, float *A, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*padded_size, A + i*n, n * sizeof(float));
}

/* Unpads C to restore matrix sizes. */
void unpad(int n, int padded_size, float *cTmp, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*n, cTmp + i*padded_size, n * sizeof(float));
}

/* C = C + AB. */
void square_sgemm(int n, float *A, float *B, float *C) {
    if (n % STRIDE == 0) {
        squarepad_sgemm(n, A, B, C);
    } else {
        const int npad = ((n + STRIDE - 1) / STRIDE) * STRIDE;
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
void squarepad_sgemm (const int n, float *A, float *B, float *C)
{
    int j_stride;
    if (n == 64)
        j_stride = STRIDE_64;
    else if (n < 768)
        j_stride = STRIDE;
    else
        j_stride = STRIDE_768;

    __m128 mmA1, mmA2, mmA3, mmA4, mmB, mmC1, mmC2, mmC3, mmC4, mmProd;
 
    for (int k = 0; k < n; k += STRIDE)
    for (int j = 0; j < n; j += j_stride)
    for (int i = 0; i < n; i += STRIDE)
        for (int k2 = k; k2 < k + STRIDE; k2++) {
            mmA1 = _mm_load_ps(A + i + k2*n);
            mmA2 = _mm_load_ps(A + i + k2*n + 4);
            mmA3 = _mm_load_ps(A + i + k2*n + 8);
            mmA4 = _mm_load_ps(A + i + k2*n + 12);
            for (int j2 = j; j2 < j + j_stride; j2++) {
                mmB = _mm_load_ps1(B + k2 + j2*n);
                
                mmC1 = _mm_load_ps(C + i + j2*n);
                mmProd = _mm_mul_ps(mmA1, mmB);
                mmC1 = _mm_add_ps(mmProd, mmC1);
                _mm_store_ps(C + i + j2*n, mmC1);

                mmC2 = _mm_load_ps(C + i + j2*n + 4);
                mmProd = _mm_mul_ps(mmA2, mmB);
                mmC2 = _mm_add_ps(mmProd, mmC2);
                _mm_store_ps(C + i + j2*n + 4, mmC2);
                
                mmC3 = _mm_load_ps(C + i + j2*n + 8);
                mmProd = _mm_mul_ps(mmA3, mmB);
                mmC3 = _mm_add_ps(mmProd, mmC3);
                _mm_store_ps(C + i + j2*n + 8, mmC3);
                    
                mmC4 = _mm_load_ps(C + i + j2*n + 12);
                mmProd = _mm_mul_ps(mmA4, mmB);
                mmC4 = _mm_add_ps(mmProd, mmC4);
                _mm_store_ps(C + i + j2*n + 12, mmC4);
            }
        }
}

