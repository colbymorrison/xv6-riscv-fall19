#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char **argv){
  for(int i = 0; i < 10; i++){
    if(!fork()){
      // Child
      int time = uptime();
      while(uptime() < time + 1000);
    }
  }
  exit(0);
}

