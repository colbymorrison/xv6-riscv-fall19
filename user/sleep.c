#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char **argv){
   if(argc < 2){
     printf("Usage: sleep [time]\n");
     exit(1);
   } 

   if(sleep(atoi(argv[1])) < 0){
        printf("Arrrgh sleep has failed");
        exit(1);
   } 

   exit(0);
}
