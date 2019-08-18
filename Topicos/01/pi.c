#include <stdio.h>
#include <math.h>

int main ()
{
  int i ;
  double pi = 0 ;
 
  for (i=0; i < 1000000; i++)
    pi += pow (-1,i) / (2*i+1) ;
  pi *= 4 ;
 
  printf ("O valor aproximado de Pi é: %f\n", pi) ;
  return (0) ;
}