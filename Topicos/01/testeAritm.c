#include "aritm.h"
#include <stdlib.h>
#include <stdio.h>

int main()
{
	int a = 10, b = 5;
	printf("Soma: %d+%d=%d\n", a, b, soma(a, b));
	printf("Subt: %d-%d=%d\n", a, b, subt(a, b));
	printf("Mult: %d*%d=%d\n", a, b, mult(a, b));
	printf("Divi: %d/%d=%d\n", a, b, divi(a, b));
	return 1;
}

