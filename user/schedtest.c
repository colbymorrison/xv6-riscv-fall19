#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char **argv){
  // Fock CPU bound 
  for(int i = 0; i < 10; i++){
    int pid = fork();
    // Spin for 1000 ticks
    if(pid ==  -1){
      printf("Fork error!\n");
      exit(1);
    }

    if(!pid){
      printf("uptime\n");
      int time = uptime();
      while(uptime() < time + 1000);
    }
  }

  //Fork io bound
  for(int i = 0; i < 20; i++){
    int pid = fork();
    if(pid == -1){
      printf("Fork error! For rd\n");
      exit(1);
    }

    if(!pid){
      printf("io\n");
      for(int i = 0; i < 1000000; i++){
        int fd;
        if((fd = open("README", 0)) == -1){
          printf("Open error!\n");
          exit(1);
        }
        char *buf[30];
        if(read(fd, buf, sizeof(buf)) == -1){
          printf("Read error!\n");
          exit(1);
        }
      }
    }
  }
  exit(0);
}

