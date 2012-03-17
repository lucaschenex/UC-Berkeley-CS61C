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
#include <string.h>

/* Converts matrix B to column-major to reduce stride */
float* transpose(int n, int paddedSize, float *B, float*dst) {
    int blocksize = 70;
    for ( int i = 0; i < n; i += blocksize ) 	
	for ( int j = 0; j < n; j += blocksize ) 
	    for ( int k = i; (k < (i + blocksize)) && (k < n); k++ ) 
		for ( int m = j; (m < (j + blocksize)) && (m < n); m++ ) {
		    dst[m*paddedSize + k] = B[m + k*n];
		}
}

/* Pads A to safely ignore matrix sizes. */
float* pad(int n, int padded_size, float *A, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*padded_size, A + i*n, n * sizeof(float));
}

/* Unpads C to restore matrix sizes. intrinsics? */
float* unpad(int n, int padded_size, float *cTmp, float *dst) {
    for (int i = 0; i < n; i++)
        memcpy(dst + i*n, cTmp + i*padded_size, n * sizeof(float));
}


/* This routine performs a sgemm operation
 *  C := C + A * B
 * where A, B, and C are lda-by-lda matrices stored in column-major format.
 * On exit, A and B maintain their input values. */    
void square_sgemm (int n, float* A, float* B, float* C)
{
    int blocksize = 16;
    int paddedSize = n;
    if (paddedSize % blocksize > 0) paddedSize += (blocksize - (paddedSize % blocksize));
    float *bTmp = calloc(paddedSize*paddedSize, sizeof(float));
    transpose(n, paddedSize, B, bTmp);
    float *aTmp = calloc(paddedSize*paddedSize, sizeof(float));
    pad(n, paddedSize, A, aTmp);
    float *cTmp = calloc(paddedSize*paddedSize, sizeof(float));

    __m128 mmA1, mmA2, mmA3, mmA4, mmB, mmC1, mmC2, mmC3, mmC4, mmTmp;
    int k, j, k2, j2, i;
    for (k = 0; k < paddedSize; k += blocksize)
	for (j = 0; j < paddedSize; j += blocksize)
	    for (k2 = k; k2 < (k + blocksize); k2++)
		for (j2 = j; j2 < (j + blocksize); j2++) {
		    mmB = _mm_load1_ps(bTmp + k2*paddedSize + j2);
		    for (i = 0; i < paddedSize; i += 16) {
			mmC1 = _mm_loadu_ps(cTmp + i + j2*paddedSize);
		    	mmA1 = _mm_loadu_ps(aTmp + i + k2*paddedSize);
			mmTmp = _mm_add_ps(mmC1, _mm_mul_ps(mmA1, mmB));
			mmA2 = _mm_loadu_ps(aTmp + i + k2*paddedSize + 4);
		    	_mm_storeu_ps(cTmp + i + j2*paddedSize, mmTmp);
		    	mmC2 = _mm_loadu_ps(cTmp + i + j2*paddedSize + 4);
			mmTmp = _mm_add_ps(mmC2, _mm_mul_ps(mmA2, mmB));
			mmA3 = _mm_loadu_ps(aTmp + i + k2*paddedSize + 8);
		    	_mm_storeu_ps(cTmp + i + j2*paddedSize + 4, mmTmp);
		    	mmC3 = _mm_loadu_ps(cTmp + i + j2*paddedSize + 8);
			mmTmp = _mm_add_ps(mmC3, _mm_mul_ps(mmA3, mmB));
			mmA4 = _mm_loadu_ps(aTmp + i + k2*paddedSize + 12);
		    	_mm_storeu_ps(cTmp + i + j2*paddedSize + 8, mmTmp);
		    	mmC4 = _mm_loadu_ps(cTmp + i + j2*paddedSize + 12);
			mmTmp = _mm_add_ps(mmC4, _mm_mul_ps(mmA4, mmB));
		    	_mm_storeu_ps(cTmp + i + j2*paddedSize + 12, mmTmp);
		    }
		}
    
    unpad(n, paddedSize, cTmp, C);

    free(aTmp); free(bTmp); free(cTmp);
}
