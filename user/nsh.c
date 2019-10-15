#include "kernel/types.h"
#include "user/user.h"

struct cmd{
  int type;
  char 

// Execute a command (child)
void execute(struct cmd*){

}

// Parsesa command
struct cmd* parsecmd(){

}

int main(int argc, char **argv){
  int bufSz = 512;
  char buf[bufSz];
  int fd;
  // Ensure we have stdin, stdout, stderr
  while((fd = open(xx, O_RDRW)) >= 0){
    if(fd >= 3){
      break;
    }
  }

  // Execute command
  while(gets(buf, bufSz)){
    // cd ?
    
    // Child executes command
    if(!fork()){
      execute(parsecmd(buf));
    } 
  }
}
