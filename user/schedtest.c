#include "kernel/types.h"
#include "user/user.h"


int main(int argc, char **argv){
  // Fork some CPU & IO bound processes 
  for(int i = 0; i < 3; i++){
    int pid = fork();
    if(pid ==  -1){
      printf("Fork error!\n");
      continue;
    }

    if(!pid){
      // Spin for 100 ticks
      int time = uptime();
      while(uptime() < time + 100);
      printf("Done! %d \n", getpid());
      exit(0);
    }
  

    pid = fork();
    if(pid == -1){
      printf("Fork error! For rd\n");
      continue;
    }

    if(!pid){
      for(int i = 0; i < 1000; i++){
        int fd;
        if((fd = open("README", 0)) == -1){
          printf("Open error\n");
          exit(1);
        }
        char buf[512];
        int rc;
        while((rc = read(fd, buf, sizeof(buf)) > 0));
        if (rc < 0) {
          printf("Read error!\n");
          exit(1);
        }
        close(fd);
      }
      exit(0);
    }
  }
  exit(0);
}
