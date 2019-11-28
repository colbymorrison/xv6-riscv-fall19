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
      // Spin for 10 ticks
      int time = uptime();
      while(uptime() < time + 10);
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
    }
  }
  exit(0);
}
