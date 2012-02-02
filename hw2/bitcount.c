/*
	Name: Benjamin Han
	Login: cs61c-mm
*/

#include <stdio.h>
#include <stdlib.h>

int bitCount(unsigned int n);

int main(int argc, char *argv[]) {
    if (argc != 2) {
	printf("usage: %s integer\n", argv[0]);
    } else {
	printf("# 1-bits in base 2 representation of %s = %d\n", argv[1], bitCount(atol(argv[1])));
	return 0;
    }
}

int bitCount(unsigned int n) {
    if (n == 0) {
	return 0;
    }
    int binaryOut = 0;
    while (n > 0) {
	if (n % 2 == 1) {	
	    binaryOut += 1;
	}
	n /= 2;
    }
    return binaryOut;
}	
