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

void square64_sgemm1 (float *A, float *B, float *C);

/* Pads A to safely ignore matrix sizes. */
void pad(int n, int padded_size, float *A, float *dst) {
    for (int i = 0; i < n/4*4; i+=4) {
        memcpy(dst + i*padded_size, A + i*n, n * sizeof(float));
	memcpy(dst + (i+1)*padded_size, A + (i+1)*n, n * sizeof(float));
	memcpy(dst + (i+2)*padded_size, A + (i+2)*n, n * sizeof(float));
	memcpy(dst + (i+3)*padded_size, A + (i+3)*n, n * sizeof(float));
    }
    for (int i = n/4*4; i < n; i++)
	memcpy(dst + i*padded_size, A + i*n, n * sizeof(float));
}

/* Unpads C to restore matrix sizes. intrinsics? */
void unpad(int n, int padded_size, float *cTmp, float *dst) {
    for (int i = 0; i < n/4*4; i+=4) {
        memcpy(dst + i*n, cTmp + i*padded_size, n * sizeof(float));
	memcpy(dst + (i+1)*n, cTmp + (i+1)*padded_size, n * sizeof(float));
	memcpy(dst + (i+2)*n, cTmp + (i+2)*padded_size, n * sizeof(float));
	memcpy(dst + (i+3)*n, cTmp + (i+3)*padded_size, n * sizeof(float));
    }
    for (int i = n/4*4; i < n; i++)
	memcpy(dst + i*n, cTmp + i*padded_size, n * sizeof(float));
	
}


/** Hard coded for 64. */
void square_sgemm (int n, float *A, float *B, float *C)
{
    const int stride = 16;
    const int npad = (n + stride - 1) & ~(stride - 1);
    const int ninit = n;

    float *Acpy, *Bcpy, *Ccpy;
    float *Cinit = C;
    if (n != npad) {
        Acpy = calloc(npad * npad, sizeof(float));
        Bcpy = calloc(npad * npad, sizeof(float));
        Ccpy = calloc(npad * npad, sizeof(float));
        pad(ninit, npad, A, Acpy);
        pad(ninit, npad, B, Bcpy);
        A = Acpy;
        B = Bcpy;
        C = Ccpy;
        n = npad;
    }
    __m128 mmA1, mmA2, mmA3, mmA4, mmB, mmC1, mmC2, mmC3, mmC4;
    __m128 mmProd1, mmProd2, mmProd3, mmProd4;
 
    for (int k = 0; k < n; k += stride)
    for (int j = 0; j < n; j += stride)
    for (int i = 0; i < n; i += stride)
        for (int k2 = k; k2 < k + stride; k2++) {
                mmA1 = _mm_load_ps(A + i + k2*n);
                mmA2 = _mm_load_ps(A + i + k2*n + 4);
                mmA3 = _mm_load_ps(A + i + k2*n + 8);
                mmA4 = _mm_load_ps(A + i + k2*n + 12);
            for (int j2 = j; j2 < j + stride; j2++) {
                mmB = _mm_load_ps1(B + k2 + j2*n);

		mmProd1 = _mm_mul_ps(mmA1, mmB);
		mmProd2 = _mm_mul_ps(mmA2, mmB);
		mmProd3 = _mm_mul_ps(mmA3, mmB);
		mmProd4 = _mm_mul_ps(mmA4, mmB);
		
                mmC1 = _mm_load_ps(C + i + j2*n);
		mmC2 = _mm_load_ps(C + i + j2*n + 4);
		mmC3 = _mm_load_ps(C + i + j2*n + 8);
		mmC4 = _mm_load_ps(C + i + j2*n + 12);
		
                mmC1 = _mm_add_ps(mmProd1, mmC1);
                mmC2 = _mm_add_ps(mmProd2, mmC2);
		mmC3 = _mm_add_ps(mmProd3, mmC3);
		mmC4 = _mm_add_ps(mmProd4, mmC4);
		
		_mm_store_ps(C + i + j2*n, mmC1);
		_mm_store_ps(C + i + j2*n + 4, mmC2);
		_mm_store_ps(C + i + j2*n + 8, mmC3);
                _mm_store_ps(C + i + j2*n + 12, mmC4);
            }
        }

    if (ninit != npad) {
        unpad(ninit, npad, Ccpy, Cinit);
        free(Acpy);
        free(Bcpy);
        free(Ccpy);
    }
}
