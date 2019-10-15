#include "kernel/types.h"
#include "user/user.h"

#define EXECCMD 1

struct cmd{
  int type;
}

struct execCmd{
  struct cmd *command;
  char *name;
  char **argv;
}


// Execute a command (child)
void execute(struct cmd*){
  switch(cmd->type){
    case EXECCMD:
      exec(cmd->name, cmd->argv) 
  }
}

// Parsesa command
struct cmd* parsecmd(){

}

int main(int argc, char **argv){
  char buf[512];
  int fd;
  // Ensure we have stdin, stdout, stderr
  while((fd = open("shell", O_RDRW)) >= 0){
    if(fd >= 3){
      break;
    }
  }

  // Execute command
  while(gets(buf, sizeof(buf))){
    // cd ?
    
    // Child executes command
    if(!fork()){
      execute(parsecmd(buf));
    } 
  }
}
