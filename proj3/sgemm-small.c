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

/* Converts matrix B to column-major to reduce stride */
float* transpose(int n, int paddedSize, int blocksize, float *B, float*dst) {
    for ( int i = 0; i < n; i += blocksize ) 	
	for ( int j = 0; j < n; j += blocksize ) 
	    for ( int k = i; (k < (i + blocksize)) && (k < n); k++ ) 
		for ( int m = j; (m < (j + blocksize)) && (m < n); m++ ) {
		    dst[m*paddedSize + k] = B[m + k*n];
		}
}

/* Pads A to ignore matrix sizes. intrinsics? */
float* pad(int n, int paddedSize, int blocksize, float *A, float *dst) {
    for ( int i = 0; i < n; i += blocksize ) 	
	for ( int j = 0; j < n; j += blocksize ) 
	    for ( int k = i; (k < (i + blocksize)) && (k < n); k++ ) 
		for ( int m = j; (m < (j + blocksize)) && (m < n); m++ ) {
		    dst[m + k*paddedSize] = A[m + k*n];
		}
}

/* Unpads C to restore matrix sizes. intrinsics? */
float* unpad(int n, int paddedSize, int blocksize, float *cTmp, float *dst) {
    for ( int i = 0; i < n; i += blocksize ) 	
	for ( int j = 0; j < n; j += blocksize ) 
	    for ( int k = i; (k < (i + blocksize)) && (k < n); k++ ) 
		for ( int m = j; (m < (j + blocksize)) && (m < n); m++ ) {
		    dst[m + k*n] = cTmp[m + k*paddedSize];
		}
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
    transpose(n, paddedSize, blocksize, B, bTmp);
    float *aTmp = calloc(paddedSize*paddedSize, sizeof(float));
    pad(n, paddedSize, blocksize, A, aTmp);
    float *cTmp = calloc(paddedSize*paddedSize, sizeof(float));

    
    __m128 mmA, mmB, mmC;
    int k, j, k2, j2, i;
    for (k = 0; k < paddedSize; k += 4)
    	for (j = 0; j < paddedSize; j += 4) {
	    mmC = _mm_loadu_ps(cTmp + i + j*n);
	    mmB = _mm_load1_ps(bTmp + k*n + j);
	    mmA = _mm_loadu_ps(aTmp + i + k*n);
	    _mm_storeu_ps(cTmp + i + j*n, _mm_add_ps(mmC, _mm_mul_ps(mmA, mmB)));
	    /* mmA = _mm_loadu_ps(aTmp + i + k*n + 4); */
	    /* mmC = _mm_loadu_ps(cTmp + i + j*n + 4); */
	    /* _mm_storeu_ps(cTmp + i + j*n + 4, _mm_add_ps(mmC, _mm_mul_ps(mmA, mmB))); */
	    /* mmA = _mm_loadu_ps(aTmp + i + k*n + 8); */
	    /* mmC = _mm_loadu_ps(cTmp + i + j*n + 8); */
	    /* _mm_storeu_ps(cTmp + i + j*n + 8, _mm_add_ps(mmC, _mm_mul_ps(mmA, mmB))); */
	    /* mmA = _mm_loadu_ps(aTmp + i + k*n + 12); */
	    /* mmC = _mm_loadu_ps(cTmp + i + j*n + 12); */
	    /* _mm_storeu_ps(cTmp + i + j*n + 12, _mm_add_ps(mmC, _mm_mul_ps(mmA, mmB))); */
	}

    unpad(n, paddedSize, blocksize, cTmp, C);

    free(aTmp); free(bTmp); free(cTmp);
}
