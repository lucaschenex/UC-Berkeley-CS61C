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
/* How much to unroll the i loop. */
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
    int blocksize = 156;
    #pragma omp parallel for
    for ( int i = 0; i < n; i += blocksize )
    for ( int j = 0; j < n; j += blocksize )
    for ( int i2 = i; i2 < i + blocksize & i2 < n; i2++ ) 
    for ( int j2 = j; j2 < j + blocksize & j2 < n; j2++ )
	dst[j2 + i2*padded_size] = src[i2 + j2*n];
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
    omp_set_num_threads(8);
    #pragma omp parallel
    {
	__m128 mmA1, mmA2, mmA3, mmA4, mmB1, mmB2, mmB3, mmB4;
	__m128 mmProd1, mmProd2, mmProd3, mmProd4, sum0, sum1, sum2, sum3;
	float tempSum0[4], tempSum1[4], tempSum2[4], tempSum3[4];

	#pragma omp for 
	for (int j = 0; j < n; j += J_STRIDE)
	for (int i = 0; i < n; i += I_STRIDE)
        for (int j2 = j; j2 < (j + J_STRIDE); j2++)
        for (int i2 = i; i2 < (i + I_STRIDE); i2 += 4) {
	    sum0 = _mm_set1_ps(0); sum1 = _mm_set1_ps(0);
	    sum2 = _mm_set1_ps(0); sum3 = _mm_set1_ps(0);
	    for (int k = 0; k < n; k += K_STRIDE) {
		mmB1 = _mm_load_ps(B + j2*n + k); //common
		mmB2 = _mm_load_ps(B + j2*n + k + 4);
		mmB3 = _mm_load_ps(B + j2*n + k + 8);
		mmB4 = _mm_load_ps(B + j2*n + k + 12);
		    
		mmA1 = _mm_load_ps(A + i2*n + k); //sum0A
		mmA2 = _mm_load_ps(A + i2*n + k + 4);
		mmA3 = _mm_load_ps(A + i2*n + k + 8);
		mmA4 = _mm_load_ps(A + i2*n + k + 12);
		    
		mmProd1 = _mm_mul_ps(mmA1, mmB1); //sum0Product
		mmProd2 = _mm_mul_ps(mmA2, mmB2);
		mmProd3 = _mm_mul_ps(mmA3, mmB3);
		mmProd4 = _mm_mul_ps(mmA4, mmB4);

		mmA1 = _mm_load_ps(A + (i2 + 1)*n + k); //sum1A
		mmA2 = _mm_load_ps(A + (i2 + 1)*n + k + 4);
		mmA3 = _mm_load_ps(A + (i2 + 1)*n + k + 8);
		mmA4 = _mm_load_ps(A + (i2 + 1)*n + k + 12);
		
		mmProd1 = _mm_add_ps(mmProd1, mmProd2); //sum0Sum
		mmProd3 = _mm_add_ps(mmProd3, mmProd4);
		mmProd1 = _mm_add_ps(mmProd1, mmProd3);
		sum0 = _mm_add_ps(mmProd1, sum0);
		    
		mmProd1 = _mm_mul_ps(mmA1, mmB1); //sum1P
		mmProd2 = _mm_mul_ps(mmA2, mmB2);
		mmProd3 = _mm_mul_ps(mmA3, mmB3);
		mmProd4 = _mm_mul_ps(mmA4, mmB4);

		mmA1 = _mm_load_ps(A + (i2 + 2)*n + k); //sum2A
		mmA2 = _mm_load_ps(A + (i2 + 2)*n + k + 4);
		mmA3 = _mm_load_ps(A + (i2 + 2)*n + k + 8);
		mmA4 = _mm_load_ps(A + (i2 + 2)*n + k + 12);

		mmProd1 = _mm_add_ps(mmProd1, mmProd2); //sum1S
		mmProd3 = _mm_add_ps(mmProd3, mmProd4);
		mmProd1 = _mm_add_ps(mmProd1, mmProd3);
		sum1 = _mm_add_ps(mmProd1, sum1);

		mmProd1 = _mm_mul_ps(mmA1, mmB1); //sum2P
		mmProd2 = _mm_mul_ps(mmA2, mmB2);
		mmProd3 = _mm_mul_ps(mmA3, mmB3);
		mmProd4 = _mm_mul_ps(mmA4, mmB4);

		mmA1 = _mm_load_ps(A + (i2 + 3)*n + k); //sum3A
		mmA2 = _mm_load_ps(A + (i2 + 3)*n + k + 4);
		mmA3 = _mm_load_ps(A + (i2 + 3)*n + k + 8);
		mmA4 = _mm_load_ps(A + (i2 + 3)*n + k + 12);

		mmProd1 = _mm_add_ps(mmProd1, mmProd2); //sum2S
		mmProd3 = _mm_add_ps(mmProd3, mmProd4);
		mmProd1 = _mm_add_ps(mmProd1, mmProd3);
		sum2 = _mm_add_ps(mmProd1, sum2);

		mmProd1 = _mm_mul_ps(mmA1, mmB1); //sum3P
		mmProd2 = _mm_mul_ps(mmA2, mmB2);
		mmProd3 = _mm_mul_ps(mmA3, mmB3);
		mmProd4 = _mm_mul_ps(mmA4, mmB4);

		mmProd1 = _mm_add_ps(mmProd1, mmProd2); //sum3S
		mmProd3 = _mm_add_ps(mmProd3, mmProd4);
		mmProd1 = _mm_add_ps(mmProd1, mmProd3);
		sum3 = _mm_add_ps(mmProd1, sum3);
			    }
	    _mm_store_ps(tempSum0, sum0); _mm_store_ps(tempSum1, sum1);
	    _mm_store_ps(tempSum2, sum2); _mm_store_ps(tempSum3, sum3);
	    C[j2*n + i2] += tempSum0[0] + tempSum0[1] + tempSum0[2] + tempSum0[3];
	    C[j2*n + i2 + 1] += tempSum1[0] + tempSum1[1] + tempSum1[2] + tempSum1[3];
	    C[j2*n + i2 + 2] += tempSum2[0] + tempSum2[1] + tempSum2[2] + tempSum2[3];
	    C[j2*n + i2 + 3] += tempSum3[0] + tempSum3[1] + tempSum3[2] + tempSum3[3];	
	    
	}
    }
}


