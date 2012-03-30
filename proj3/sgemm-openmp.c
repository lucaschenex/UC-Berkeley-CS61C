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
#include <omp.h>

/* Determines cache blocking (must divide 16 in the general case). */
#define BLOCK 16
#define BLOCK_64 64
#define BLOCK_512 8
#define BLOCK_768 4

/* How much to unroll the i loop. */
#define I_STRIDE 16
/* How much to unroll the k loop. */
#define K_STRIDE 16
#define J_STRIDE 16

/* OPENMP settings. */
#define NUM_THREADS 8

/* Forward declaration. */
void squarepad_sgemm(int n, float *A, float *B, float *C);

/* Pads src to safely ignore matrix sizes. */
inline void pad(int n, int padded_size, float *src, float *dst) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++)
        memcpy(dst + i*padded_size, src + i*n, n * sizeof(float));
}

void transpose(int n, int padded_size, float *src, float *dst) {
    #pragma omp parallel for
    for ( int i = 0; i < n; i++ )
    for ( int j = 0; j < n; j++ )
	dst[j + i*padded_size] = src[i + j*n];
}

/* Unpads src to restore matrix sizes. */
inline void unpad(int n, int padded_size, float *src, float *dst) {
    #pragma omp parallel for
    for (int i = 0; i < n; i++)
        memcpy(dst + i*n, src + i*padded_size, n * sizeof(float));
}

/* C = C + AB. */
void square_sgemm(int n, float *A, float *B, float *C) {
    const int npad = ((n + K_STRIDE - 1) / K_STRIDE) * K_STRIDE;
    float *Acpy = calloc(npad * npad, sizeof(float));
    transpose(n, npad, A, Acpy);
    if (n % I_STRIDE == 0) {
	squarepad_sgemm(n, Acpy, B, C);
    } else {
	float *Bcpy = calloc(npad * npad, sizeof(float));
	float *Ccpy = calloc(npad * npad, sizeof(float));
        
	transpose(n, npad, A, Acpy);
	pad(n, npad, B, Bcpy);
	pad(n, npad, C, Ccpy);
    
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

    #pragma omp parallel
    {
	__m128 mmA1, mmA2, mmA3, mmA4, mmB1, mmB2, mmB3, mmB4, sum;
	__m128 mmProd1, mmProd2, mmProd3, mmProd4;
	float tempSum[4];

	#pragma omp for 
	for (int j = 0; j < n; j += J_STRIDE)
	for (int i = 0; i < n; i += I_STRIDE)
	for (int j2 = j; j2 < (j + J_STRIDE); j2++)
        for (int i2 = i; i2 < (i + I_STRIDE); i2++) {
	    sum = _mm_set1_ps(0);
	    for (int k = 0; k < n; k += K_STRIDE) {
		mmB1 = _mm_load_ps(B + j2*n + k);
		mmB2 = _mm_load_ps(B + j2*n + k + 4);
		mmB3 = _mm_load_ps(B + j2*n + k + 8);
		mmB4 = _mm_load_ps(B + j2*n + k + 12);
		    
		mmA1 = _mm_load_ps(A + i2*n + k);
		mmA2 = _mm_load_ps(A + i2*n + k + 4);
		mmA3 = _mm_load_ps(A + i2*n + k + 8);
		mmA4 = _mm_load_ps(A + i2*n + k + 12);
		    
		mmProd1 = _mm_mul_ps(mmA1, mmB1);
		mmProd2 = _mm_mul_ps(mmA2, mmB2);
		mmProd3 = _mm_mul_ps(mmA3, mmB3);
		mmProd4 = _mm_mul_ps(mmA4, mmB4);

		mmProd1 = _mm_add_ps(mmProd1, mmProd2);
		mmProd3 = _mm_add_ps(mmProd3, mmProd4);
		mmProd1 = _mm_add_ps(mmProd1, mmProd3);
		sum = _mm_add_ps(mmProd1, sum);
	    }
	    _mm_store_ps(tempSum, sum);
	    C[j2*n + i2] += tempSum[0] + tempSum[1] + tempSum[2] + tempSum[3];	
	}
    }
}


