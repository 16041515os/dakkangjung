#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

int main(int argc, char** argv){

  int num1,num2,num3,num4;
  int res1 = 0, res2 = 0;


  if(argc == 5){
    num1 = atoi(argv[1]); 
    num2 = atoi(argv[2]);
    num3 = atoi(argv[3]);
    num4 = atoi(argv[4]);

    res1 = pibonacci(num1);
    res2 = sum_of_four_integers(num1,num2,num3,num4);
  
    printf("%d %d\n", res1, res2);
  }
 
  return EXIT_SUCCESS;
}
