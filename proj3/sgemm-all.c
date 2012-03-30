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
#include <string.h>

/* Determines cache blocking (must divide 16 in the general case). */
#define BLOCK_BIG 16
#define BLOCK_MED 8
#define BLOCK_SML 4

/* How much to unroll the k loop. */
#define K_STRIDE 16

/* Forward declaration. */
void squarepad_sgemm(int n, float *A, float *B, float *C);

/* Pads src to safely ignore matrix sizes. */
inline void pad(int n, int padded_size, float *src, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*padded_size, src + i*n, n * sizeof(float));
}

void transpose(int n, int padded_size, float *src, float *dst) {
    int blocksize = 128;
    for ( int i = 0; i < n; i += blocksize )
    for ( int j = 0; j < n; j += blocksize )
    for ( int i2 = i; i2 < i + blocksize & i2 < n; i2++ ) 
    for ( int j2 = j; j2 < j + blocksize & j2 < n; j2++ )
	dst[j2 + i2*padded_size] = src[i2 + j2*n];
}

/* Unpads src to restore matrix sizes. */
inline void unpad(int n, int padded_size, float *src, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*n, src + i*padded_size, n * sizeof(float));
}

/* C = C + AB. */
void square_sgemm(int n, float *A, float *B, float *C) {
    const int npad = ((n + K_STRIDE - 1) / K_STRIDE) * K_STRIDE;
    float *Acpy = calloc(npad * npad, sizeof(float));
    transpose(n, npad, A, Acpy);
    if (n % K_STRIDE == 0) {
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
	free(Bcpy);
	free(Ccpy);
    }
    free(Acpy);
}

/** Assumes input matrix has dimension n divisible by STRIDE. */
inline void squarepad_sgemm (const int n, float *A, float *B, float *C) {
    __m128 mmA1, mmA2, mmA3, mmA4, mmB1, mmB2, mmB3, mmB4, mmC;
    __m128 mmProd1, mmProd2, mmProd3, mmProd4;
    __m128 sum0, sum1, sum2, sum3, sum4, sum5, sum6, sum7;
    float tempSum0[4], tempSum1[4], tempSum2[4], tempSum3[4], tempSum4[4];
    float tempSum5[4], tempSum6[4], tempSum7[4];
    int J_STRIDE, I_STRIDE;
    
    if (n < 512) {
	J_STRIDE = BLOCK_BIG;
	I_STRIDE = BLOCK_BIG;
    } else if (n < 768) {
	I_STRIDE = BLOCK_MED;
	J_STRIDE = BLOCK_BIG;
    } else if (n < 801) {
	I_STRIDE = BLOCK_MED;
	J_STRIDE = BLOCK_MED;
    } else {
	I_STRIDE = BLOCK_MED;
	J_STRIDE = BLOCK_SML;
    }
    
    for (int j = 0; j < n; j += J_STRIDE)
    for (int i = 0; i < n; i += I_STRIDE)
    for (int j2 = j; j2 < (j + J_STRIDE); j2++)
    for (int i2 = i; i2 < (i + I_STRIDE); i2 += 8) {
	sum0 = _mm_set1_ps(0); sum1 = _mm_set1_ps(0);
	sum2 = _mm_set1_ps(0); sum3 = _mm_set1_ps(0);
	sum4 = _mm_set1_ps(0); sum5 = _mm_set1_ps(0);
	sum6 = _mm_set1_ps(0); sum7 = _mm_set1_ps(0);
	for (int k = 0; k < n; k += K_STRIDE) {
	    mmB1 = _mm_load_ps(B + j2*n + k); //Bload
	    mmB2 = _mm_load_ps(B + j2*n + k + 4);
	    mmB3 = _mm_load_ps(B + j2*n + k + 8);
	    mmB4 = _mm_load_ps(B + j2*n + k + 12);
		    
	    mmA1 = _mm_load_ps(A + i2*n + k); //0ALoad
	    mmA2 = _mm_load_ps(A + i2*n + k + 4);
	    mmA3 = _mm_load_ps(A + i2*n + k + 8);
	    mmA4 = _mm_load_ps(A + i2*n + k + 12);
		    
	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //0Product
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmA1 = _mm_load_ps(A + (i2 + 1)*n + k); //1A
	    mmA2 = _mm_load_ps(A + (i2 + 1)*n + k + 4);
	    mmA3 = _mm_load_ps(A + (i2 + 1)*n + k + 8);
	    mmA4 = _mm_load_ps(A + (i2 + 1)*n + k + 12);
		
	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //0Sum
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum0 = _mm_add_ps(mmProd1, sum0);
		    
	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //1P
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmA1 = _mm_load_ps(A + (i2 + 2)*n + k); //2A
	    mmA2 = _mm_load_ps(A + (i2 + 2)*n + k + 4);
	    mmA3 = _mm_load_ps(A + (i2 + 2)*n + k + 8);
	    mmA4 = _mm_load_ps(A + (i2 + 2)*n + k + 12);

	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //1S
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum1 = _mm_add_ps(mmProd1, sum1);

	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //2P
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmA1 = _mm_load_ps(A + (i2 + 3)*n + k); //3A
	    mmA2 = _mm_load_ps(A + (i2 + 3)*n + k + 4);
	    mmA3 = _mm_load_ps(A + (i2 + 3)*n + k + 8);
	    mmA4 = _mm_load_ps(A + (i2 + 3)*n + k + 12);

	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //2S
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum2 = _mm_add_ps(mmProd1, sum2);

	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //3P
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmA1 = _mm_load_ps(A + (i2 + 4)*n + k); //4A
	    mmA2 = _mm_load_ps(A + (i2 + 4)*n + k + 4);
	    mmA3 = _mm_load_ps(A + (i2 + 4)*n + k + 8);
	    mmA4 = _mm_load_ps(A + (i2 + 4)*n + k + 12);

	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //3S
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum3 = _mm_add_ps(mmProd1, sum3);

	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //4P
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmA1 = _mm_load_ps(A + (i2 + 5)*n + k); //5A
	    mmA2 = _mm_load_ps(A + (i2 + 5)*n + k + 4);
	    mmA3 = _mm_load_ps(A + (i2 + 5)*n + k + 8);
	    mmA4 = _mm_load_ps(A + (i2 + 5)*n + k + 12);

	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //4S
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum4 = _mm_add_ps(mmProd1, sum4);

	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //5P
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmA1 = _mm_load_ps(A + (i2 + 6)*n + k); //6A
	    mmA2 = _mm_load_ps(A + (i2 + 6)*n + k + 4);
	    mmA3 = _mm_load_ps(A + (i2 + 6)*n + k + 8);
	    mmA4 = _mm_load_ps(A + (i2 + 6)*n + k + 12);

	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //5S
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum5 = _mm_add_ps(mmProd1, sum5);

	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //6P
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmA1 = _mm_load_ps(A + (i2 + 7)*n + k); //7A
	    mmA2 = _mm_load_ps(A + (i2 + 7)*n + k + 4);
	    mmA3 = _mm_load_ps(A + (i2 + 7)*n + k + 8);
	    mmA4 = _mm_load_ps(A + (i2 + 7)*n + k + 12);

	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //6S
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum6 = _mm_add_ps(mmProd1, sum6);

	    mmProd1 = _mm_mul_ps(mmA1, mmB1); //7P
	    mmProd2 = _mm_mul_ps(mmA2, mmB2);
	    mmProd3 = _mm_mul_ps(mmA3, mmB3);
	    mmProd4 = _mm_mul_ps(mmA4, mmB4);

	    mmProd1 = _mm_add_ps(mmProd1, mmProd2); //7S
	    mmProd3 = _mm_add_ps(mmProd3, mmProd4);
	    mmProd1 = _mm_add_ps(mmProd1, mmProd3);
	    sum7 = _mm_add_ps(mmProd1, sum7);	    
	}
	sum0 = _mm_hadd_ps(sum0, sum1);
	sum2 = _mm_hadd_ps(sum2, sum3);
	sum0 = _mm_hadd_ps(sum0, sum2);

	sum4 = _mm_hadd_ps(sum4, sum5);
	sum6 = _mm_hadd_ps(sum6, sum7);
	sum4 = _mm_hadd_ps(sum4, sum6);

	mmC = _mm_load_ps(C + j2*n + i2);
	mmC = _mm_add_ps(mmC, sum0);
	_mm_store_ps(C + j2*n + i2, mmC);

	mmC = _mm_load_ps(C + j2*n + i2 + 4);
	mmC = _mm_add_ps(mmC, sum4);
	_mm_store_ps(C + j2*n + i2 + 4, mmC);
    }
}


