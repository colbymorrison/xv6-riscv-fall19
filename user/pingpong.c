#include "kernel/types.h"
#include "user/user.h"

void createPipe(int *pipefd){
    if(pipe(pipefd) < 0){
      printf("Pipe error!");
      exit(1);
    }
}

int main(int argc, char **argv){
    int pipe0[2];
    int pipe1[2];
    int child;

    createPipe(pipe0);
    createPipe(pipe1);

    if((child = fork()) < 0){
     printf("Fork error!");
     exit(1);
    } 

    //Child
    if(!child){
      uchar cByte; 
      read(pipe0[0], &cByte, 1);
      printf("%d: recieved ping\n", getpid());
      write(pipe1[1], &cByte, 1);
    }
    // Parent
    else{
      uchar pByte = 1;
      write(pipe0[1], &pByte, 1);
      read(pipe1[0], &pByte, 1);
      printf("%d: received pong\n", getpid());
    }
    exit(0);
} 
