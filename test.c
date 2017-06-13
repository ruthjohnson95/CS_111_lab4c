#include <stdlib.h>
#include <stdio.h>


int main()
{
  FILE *fp;
  fp=fopen("log.txt", "w");
  fprintf(fp, "Testing...\n");
  return 0;

}
